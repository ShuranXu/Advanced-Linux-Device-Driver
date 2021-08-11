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
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/workqueue.h>

MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");
MODULE_AUTHOR("Daniele Paolo Scarpazza");
MODULE_LICENSE("GPL");


struct tty_driver *my_driver;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
struct timer_list my_timer;
char kbledstatus = 0;
#else
typedef struct 
{
	char kbledstatus;
	struct timer_list my_timer;
}blinkplus_t;

blinkplus_t blinkplus;
#endif 

#define BLINK_DELAY   HZ/5
#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF

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

	int *pstatus = (int *)ptr;

	if (*pstatus == ALL_LEDS_ON)
		*pstatus = RESTORE_LEDS;
	else
		*pstatus = ALL_LEDS_ON;

	/** vc_cons is a struct type vc that contains a pointer to a 
	  * virtual console (d) of type vc_data	vc_tty is the tty port where 
	  * console is attached, fg_console is the current console 
	  * vt_ioctl(struct tty_struct *tty, struct file *file, 	
	  *                         unsigned int cmd, unsigned long arg)
	  */
	
	// calling ioctl in timer function may result in hang. -- FIX IT
//	(my_driver->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, *pstatus);

	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);

	my_timer.expires = jiffies + BLINK_DELAY;
    printk("my_timer_func, %d", *pstatus);
	add_timer(&my_timer);
}
#else 
static void my_timer_func(struct timer_list * data){

	blinkplus_t *blinkplusptr = container_of(data, blinkplus_t, my_timer);
	int *pstatus = (int*)blinkplusptr->kbledstatus;

	if (*pstatus == ALL_LEDS_ON)
		*pstatus = RESTORE_LEDS;
	else
		*pstatus = ALL_LEDS_ON;

	// calling ioctl in timer function may result in hang. -- FIX IT
	// (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);

	blinkplus.my_timer.expires = jiffies + BLINK_DELAY;
    printk("my_timer_func, %d", *pstatus);
	add_timer(&blinkplus.my_timer);
}
#endif 

static ssize_t blink_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

	printk("blinkplus: ioctl\n");
	return 0;
}

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

	/*
	 * Set up the LED blink timer the first time
	 */
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	init_timer(&my_timer);
	my_timer.function = my_timer_func;
	my_timer.data = (unsigned long)&kbledstatus;
	my_timer.expires = jiffies + BLINK_DELAY;
	add_timer(&my_timer);
	#else
	blinkplus.my_timer.expires = jiffies + BLINK_DELAY;  
	timer_setup(&blinkplus.my_timer, my_timer_func, 0); 
	#endif 

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

	printk(KERN_INFO "blinkplus: %d\n",__LINE__);
	// Create a device class 
	blink_class = class_create(THIS_MODULE,mydev_name);
	if (IS_ERR(blink_class)) {
		printk(KERN_ERR "Error creating hello class.\n");
		result = PTR_ERR(blink_class);
		cdev_del(blink_cdev);
		unregister_chrdev_region(dev, 1);
		return -1;
    }
	// Create and add device under blink_class
    device_create(blink_class,NULL,dev,NULL,"blinkplus%d",0);

	printk(KERN_INFO "blinkplus: %d\n",__LINE__);
	printk(KERN_INFO "blinkplus: loading\n");

	return 0;
}

static void __exit kbleds_cleanup(void){

	printk(KERN_INFO "kbleds: unloading...\n");
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	del_timer(&my_timer);
	#else
	del_timer(&blinkplus.my_timer);
	#endif 
	(my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);

	cdev_del(blink_cdev);
	device_destroy(blink_class, dev);
	class_destroy(blink_class);
	unregister_chrdev_region(dev,1);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);

