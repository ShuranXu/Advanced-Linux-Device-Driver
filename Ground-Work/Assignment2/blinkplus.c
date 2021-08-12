/* 
 *  kbleds.c - Blink keyboard leds until the module is unloaded.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/tty.h>		/* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>		/* For KDSETLED */
#include <linux/vt_kern.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include<linux/slab.h> // for kmalloc
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/workqueue.h>


#include <linux/smp.h>


MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");
MODULE_AUTHOR("Daniele Paolo Scarpazza");
MODULE_LICENSE("GPL");

#define BLINK_DELAY   	HZ/5
#define LED_SCROLL      0x01UL
#define LED_NUML        0x02UL
#define LED_CAPSL       0x04UL
#define ALL_LEDS_ON     0x07UL
#define RESTORE_LEDS    0xFFUL

				
#define MAGIC 		'z'
#define GETINV      _IOR(MAGIC, 1, int *)
#define SETLED      _IOW(MAGIC, 2, int *)
#define SETINV      _IOW(MAGIC, 3, int *)


struct tty_driver *my_driver;

typedef struct blink_s {
	unsigned int               status;
    unsigned long              blink_delay;
    struct work_struct         wk;    // work
    struct workqueue_struct    *wq;   // work queue
} blink_info_t;	

typedef struct 
{
	unsigned long kbledstatus;
	struct timer_list my_timer;
	blink_info_t info;
}blinkplus_t;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
struct timer_list my_timer;
blink_info_t info;
unsigned long kbledstatus = 0;
#else
static blinkplus_t blinkplus;
#endif 

static DEFINE_SPINLOCK(lock);

/* This is to create /dev/blinkplus device nodes */

static char mydev_name[]="blinkplus";  // This will appears in /proc/devices
static struct cdev  *blink_cdev;
static struct class *blink_class;
static dev_t  dev;

static int blink_open(struct inode *inode, struct file *file);
static ssize_t blink_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int blink_release(struct inode *inode, struct file *file);

static struct file_operations blink_fops = {
	open: blink_open,
	release: blink_release,
	unlocked_ioctl: blink_ioctl,
	owner:	 THIS_MODULE
};

/*
 * Function my_timer_func blinks the keyboard LEDs periodically by invoking
 * command KDSETLED of ioctl() on the keyboard driver. To learn more on virtual 
 * terminal ioctl operations, please see file:
 *     /usr/src/linux/drivers/char/vt_ioctl.c, function vt_ioctl().
 *
 * The argument to KDSETLED is alternatively set to 7 (thus causing the led 
 * mode to be set to LED_SHOW_IOCTL, and all the leds are lit) and to 0xFF
 * (any value above 7 switches back the led mode to LED_SHOW_FLAGS, thus
 * the LEDs reflect the actual keyboard status).  To learn more on this, 
 * please see file:
 *     /usr/src/linux/drivers/char/keyboard.c, function setledstate().
 * 
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void my_timer_func(unsigned long ptr){

	blink_info_t *info = container_of(ptr, blink_info_t, blink_delay);
	my_timer.expires = jiffies + info->blink_delay;
	printk(KERN_INFO "in my_timer_func - %lu, info: %d\n", jiffies, info->status);
	// schedule_work (info->wk);   // system wide work queue
    // For dedicated work queue
	queue_work(info->wq, info->wk);
	add_timer(&my_timer);
	printk(KERN_INFO "blinkplus: timer %lu %lu\n",jiffies,my_timer.expires);
}
#else 
static void my_timer_func(struct timer_list *ptr){

	blinkplus_t *blinkplusptr = container_of(ptr, blinkplus_t, my_timer);
	blinkplusptr->my_timer.expires = jiffies + blinkplusptr->info.blink_delay;

	// printk(KERN_INFO "my_timer_func - running CPU : %d\n", get_cpu());
	printk(KERN_INFO "in my_timer_func - %lu, info: %d\n", jiffies, blinkplusptr->info.status);
	// schedule_work (blinkplusptr.info->wk);   // system wide work queue
    // For dedicated work queue
    queue_work(blinkplusptr->info.wq, &blinkplusptr->info.wk);
	add_timer(&blinkplusptr->my_timer);
	printk(KERN_INFO "blinkplus: timer %lu %lu\n",jiffies,blinkplusptr->my_timer.expires);
}
#endif 

/*
 * Write the work function that invokes the ioctl instead of running it 
 * from timer function
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void do_work(struct work_struct *work){

	blink_info_t *info = container_of(&work, blink_info_t, wk);
	unsigned int *pstatus = (unsigned int *)&info->status;

	if (*pstatus == ALL_LEDS_ON)
		*pstatus = RESTORE_LEDS;
	else
		*pstatus = kbledstatus & ALL_LEDS_ON;

	printk (KERN_INFO "\n *pstatus:%d  ledstatus:%ld", *pstatus, kbledstatus);
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);
}
#else
static void do_work(struct work_struct *work){

	unsigned long flags;
	/* Serialize access */
  	spin_lock_irqsave(&lock, flags);

	blink_info_t *info = container_of(work, blink_info_t, wk);
	unsigned int *pstatus = (unsigned int *)&info->status;

	if (*pstatus == ALL_LEDS_ON)
		*pstatus = RESTORE_LEDS;
	else
		*pstatus = blinkplus.kbledstatus & ALL_LEDS_ON;

	printk(KERN_INFO "\n *pstatus:%d  ledstatus:%ld", *pstatus, blinkplus.kbledstatus);
	spin_unlock_irqrestore(&lock, flags);
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static ssize_t blink_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

	switch(cmd){
		case GETINV:
			if (!arg)
				return -EINVAL;
			printk("GETINV was issued \n");
			if (copy_to_user((long *)arg, &info.blink_delay, sizeof(long)))
				return -EFAULT;
			printk( "helloplus: GETINV returns :%ld\n",info.blink_delay);
			return 0;
		case SETINV:
			if (!arg)
				return -EINVAL;
			/*
			*  New timer is requested by user. This version guarantees that 
			*  the timer function itself is not running when it returns.
			*  This will avoid any race condition in smp environment
			*/
		
			if (copy_from_user(&info.blink_delay, (long *) arg,  sizeof (long)))
				return -EFAULT;
			printk(KERN_INFO "SETINV set to:  %ld\n", info.blink_delay);
			del_timer_sync(&my_timer);
			my_timer.function = my_timer_func;
			my_timer.data = &info.blink_delay;
			my_timer.expires = jiffies + info.blink_delay;
			add_timer(&my_timer);
	
			return 0;
		case SETLED:
			if (!arg)
				return -EINVAL;
			if (copy_from_user(&kbledstatus, (unsigned long *) arg,  sizeof (unsigned long)))
				return -EFAULT;
			printk(KERN_INFO "SETLED set to:  %ld\n", kbledstatus);
			return 0;

		default:  	// unknown command 
			return -ENOTTY;
	}
    
	return -ENOTTY;
}
#else
static ssize_t blink_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

	switch(cmd){
		case GETINV:
			if (!arg)
				return -EINVAL;
			printk("GETINV was issued \n");
			if (copy_to_user((long *)arg, &blinkplus.info.blink_delay, sizeof(long)))
				return -EFAULT;
			printk( "helloplus: GETINV returns :%ld\n",blinkplus.info.blink_delay);
			return 0;
		case SETINV:
			if (!arg)
				return -EINVAL;
			if (copy_from_user(&blinkplus.info.blink_delay, (long *) arg,  sizeof (long)))
				return -EFAULT;
			/*
			*  New timer is requested by user. This version guarantees that 
			*  the timer function itself is not running when it returns.
			*  This will avoid any race condition in smp environment
			*/
			
			printk(KERN_INFO "SETINV set to:  %ld\n", blinkplus.info.blink_delay);
			del_timer_sync(&blinkplus.my_timer);
			blinkplus.my_timer.expires = jiffies + blinkplus.info.blink_delay;  
			add_timer(&blinkplus.my_timer);
			return 0;
		case SETLED:
			if (!arg)
				return -EINVAL;
			if (copy_from_user(&blinkplus.kbledstatus, (unsigned long *) arg,  sizeof (unsigned long))){
				return -EFAULT;
			}
			printk(KERN_INFO "SETLED set to:  %ld\n", blinkplus.kbledstatus);
			return 0;

		default:  	// unknown command 
			return -ENOTTY;
	}
	return -ENOTTY;
}
#endif 

static int blink_open(struct inode *inode, struct file *file){
	printk("blinkplus: open\n");
	return 0;
}

static int blink_release(struct inode *inode, struct file *file){
	printk("blinkplus: release\n");
	return 0;
}

static int __init kbleds_init(void){

	int i;
	int result;
	int major;

	printk(KERN_INFO "kbleds: loading\n");
	printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
	for (i = 0; i < MAX_NR_CONSOLES; i++) {
		if (!vc_cons[i].d)
			break;
		printk(KERN_INFO " console[%i/%i] #%i, tty %lx\n", i,
		       MAX_NR_CONSOLES, vc_cons[i].d->vc_num, (unsigned long)vc_cons[i].d->port.tty);
		}
	printk(KERN_INFO "kbleds: finished scanning consoles\n");

	my_driver = vc_cons[fg_console].d->port.tty->driver;
	printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);

	result = alloc_chrdev_region(&dev, 0, 1, mydev_name);
	if (result<0) 
		return result;

    major = MAJOR(dev);

	printk("The device is registered by Major no: %d\n", major);

	// Allocate a cdev structure 
    blink_cdev = cdev_alloc();
	
	// Attach hello fops methods with the cdev: blink_cdev->ops=&blink_fops 
	cdev_init (blink_cdev, &blink_fops);
    blink_cdev->owner = THIS_MODULE;

	// Connect the assigned major number to the cdev 
    result = cdev_add(blink_cdev, dev, 1);
    if (result<0){
		printk("Error in registering the module\n");
        unregister_chrdev_region(dev, 1);
        return result;
    }

	// Create a device class 
	blink_class = class_create(THIS_MODULE,mydev_name);
	if (IS_ERR(blink_class)) {
		printk(KERN_ERR "Error creating hello class.\n");
		result = PTR_ERR(blink_class);
		cdev_del(blink_cdev);
		unregister_chrdev_region(dev, 1);
		return result;
    }
	// Create and add device under blink_class
    device_create(blink_class,NULL,dev,NULL,"blinkplus");

	/*
	 * Set up the LED blink timer the first time
	 */
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	init_timer(&my_timer);
	my_timer.function = my_timer_func;
	info.blink_delay = BLINK_DELAY;
	info.status = 0;
	my_timer.data = (unsigned long)&info.blink_delay;
	my_timer.expires = jiffies + BLINK_DELAY;
	add_timer(&my_timer);
	// Initialize work queue and bind do_work() with info.wq
	info.wq = create_singlethread_workqueue("blinkdrv");
	if(!info.wq){
		printk(KERN_ERR "Error creating workqueue.\n");
		cdev_del(blink_cdev);
		unregister_chrdev_region(dev, 1);
		return -1;
	}
	INIT_WORK(&info.wk, do_work);
	#else
	blinkplus.info.blink_delay = BLINK_DELAY;
	blinkplus.my_timer.expires = jiffies + BLINK_DELAY;  
	timer_setup(&blinkplus.my_timer, my_timer_func, 0); 
	/* setup timer interval to based on TIMEOUT Macro */
    mod_timer(&blinkplus.my_timer, jiffies + BLINK_DELAY);

	// Initialize work queue and bind do_work() with info.wq
	blinkplus.info.wq = create_singlethread_workqueue("blinkdrv");
	if(!blinkplus.info.wq){
		printk(KERN_ERR "Error creating workqueue.\n");
		cdev_del(blink_cdev);
		unregister_chrdev_region(dev, 1);
		return -1;
	}

	INIT_WORK(&blinkplus.info.wk, do_work);
	#endif 

	printk(KERN_INFO "blinkplus: %d\n",__LINE__);
	printk(KERN_INFO "blinkplus: loading\n");
	return 0;
}

static void __exit kbleds_cleanup(void){

	printk(KERN_INFO "kbleds: unloading...\n");
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	del_timer(&my_timer);
	cancel_work_sync(&info.wk);
	destroy_workqueue(info.wq);
	#else
	del_timer(&blinkplus.my_timer);
	cancel_work_sync(&blinkplus.info.wk);
	destroy_workqueue(blinkplus.info.wq);
	#endif 
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);

	cdev_del(blink_cdev);
	device_destroy(blink_class, dev);
	class_destroy(blink_class);
	unregister_chrdev_region(dev,1);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);

