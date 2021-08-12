/* 
 *  mmap.c - mmap implementation with kmalloc and vmalloc
 */

#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/kdev_t.h>
#include <linux/proc_fs.h>
#include <linux/device.h>


MODULE_DESCRIPTION("Mmap Implementation Demo");
MODULE_AUTHOR("Shuran Xu");
MODULE_LICENSE("GPL");

#define LEN (16*1024) 

static char *kmalloc_area = NULL;
static char *kmalloc_ptr = NULL;
unsigned long virt_addr;
static char *vmalloc_ptr = NULL;


/* This is to create /dev/mmaper device nodes */
static char dev_name[]="mmaper";  
static struct cdev  *mmaper_cdev;
static struct class *mmaper_class;
static dev_t dev;
// static struct proc_dir_entry *proc_mapper;


static int mmap_kmalloc(struct file * filp, struct vm_area_struct * vma) {

    int ret;
    unsigned long length;
    length = vma->vm_end - vma->vm_start;

	// Restrict to size of device memory
    if (length > LEN * PAGE_SIZE)
        return -EIO;

	/**
     * remap_pfn_range function arguments:
     * vma: vm_area_struct has passed to the mmap method
     * vma->vm_start: start of mapping user address space
     * Page frame number of first page that you can get by:
     *   virt_to_phys((void *)kmalloc_area) >> PAGE_SHIFT
     * size: length of mapping in bytes which is simply vm_end - vm_start
     * vma->>vm_page_prot: protection bits received from the application
     */

    vma->vm_flags |= VM_RESERVED;
    ret = remap_pfn_range(
        vma, 
        vma->vm_start,
            virt_to_phys((void*)((unsigned long)kmalloc_area)) >> PAGE_SHIFT,
            vma->vm_end-vma->vm_start,
        vma->vm_page_prot 
            );
    if(ret != 0) {
            return -EAGAIN;
    }
    return 0;
}

static int mmap_vmem(struct file *filp, struct vm_area_struct *vma){

    int ret;
    long length = vma->vm_end - vma->vm_start;
    unsigned long start = vma->vm_start;
    char *vmalloc_area_ptr = (char *)vmalloc_ptr;
    unsigned long pfn;

    /* Restrict it to size of device memory */
    if (length > LEN * PAGE_SIZE)
            return -EIO;

    /** 
	* Considering vmalloc pages are not contiguous in physical memory
    * You need to loop over all pages and call remap_pfn_range 
	* for each page individuallay. Also, use 
    * vmalloc_to_pfn(vmalloc_area_prt)
	* instead to get the page frame number of each virtual page
	*/
    vma->vm_flags |= VM_RESERVED;
    while (length > 0) {
            pfn = vmalloc_to_pfn(vmalloc_area_ptr);
            printk("vmalloc_area_ptr: 0x%p \n", vmalloc_area_ptr);

            if ((ret = remap_pfn_range(vma, start, pfn, PAGE_SIZE,
                                        vma->vm_page_prot)) < 0) {
                    return ret;
            }
            start += PAGE_SIZE;
            vmalloc_area_ptr += PAGE_SIZE;
            length -= PAGE_SIZE;
    }
    return 0;
}

static int mmap_kmalloc_init(void) {

    int i;
    int ret;

    /**
     * kmalloc() returns memory in bytes instead of PAGE_SIZE
     * mmap memory should be PAGE_SIZE and aligned on a PAGE boundary.
     */

    kmalloc_ptr = kmalloc(LEN + (2 * PAGE_SIZE), GFP_KERNEL);
    if (!kmalloc_ptr) {
            printk("kmalloc failed\n");
            return -ENOMEM;
    }
    printk("kmalloc_ptr at 0x%p \n", kmalloc_ptr);

    /**
      * This is the same as: 
      * (int *)((((unsigned long)kmalloc_ptr) + ((1<<12) - 1)) & 0xFFFF0000);
      * where: PAGE_SIZE is defined as 1UL <<PAGE_SHIFT. 
      * That is 4k on x86. 0xFFFF0000 is a PAGE_MASK to mask out the upper 
      * bits in the page. This will align it at 4k page boundary that means 
      * kmalloc start address is now page aligned.
      */

    kmalloc_area = (char *)(((unsigned long)kmalloc_ptr + PAGE_SIZE -1) & PAGE_MASK);

    printk("kmalloc_area: 0x%p\n", kmalloc_area);

	/** 
     * reserve kmalloc memory as pages to make them remapable
     * Since the pages are mapped to user space, they might be swapped out. 
     * To avoid this we must set the PG_reserved bit on the page. Enabling is done 
     * using SetPageReserved() while reseting it (which must be done before freeing 
     * the memory) is done with ClearPageReserved().
    */
    for (virt_addr=(unsigned long)kmalloc_area; virt_addr < (unsigned long)kmalloc_area + LEN;
            virt_addr+=PAGE_SIZE) {
                    SetPageReserved(virt_to_page(virt_addr));
    }
    printk("kmalloc_area: 0x%p\n" , kmalloc_area);
    printk("kmalloc_area :0x%p \t physical Address 0x%lx)\n", kmalloc_area,
                        virt_to_phys((void *)(kmalloc_area)));

    /**
     *  Write code to init memory with ascii 0123456789. Where ascii 
     *  equivalent of 0 is 48  and 9 is 58. This is read from mmap() by 
     *  user level application
     */
    for(i=0;i<10;i++){
            kmalloc_area[i] = 48 + i;
    }
    
    return 0;
}

static int mmap_vmem_init(void) {

    unsigned long virt_addr;

	/* Allocate  memory  with vmalloc. It is already page aligned */
    vmalloc_ptr = ((char *)vmalloc(LEN);
    if (!vmalloc_ptr) {
            printk("vmalloc failed\n");
            return -ENOMEM;
    }
    printk("vmalloc_ptr at 0x%p \n", vmalloc_ptr);

    /* reserve vmalloc memory to make them remapable */
    for (virt_addr=(unsigned long)vmalloc_ptr; 
        virt_addr < (unsigned long)vmalloc_ptr + LEN; 
        virt_addr+=PAGE_SIZE) {
                        SetPageReserved(vmalloc_to_page((unsigned long *)virt_addr));
                    }
    printk("vmalloc_ptr: 0x%p\n" , vmalloc_ptr);
    printk("vmalloc_ptr :0x%p \t physical Address 0x%lx)\n", vmalloc_ptr,
                        virt_to_phys((void *)(vmalloc_ptr)));
    /**
	*  Initialize memory with "abcdefghijklmnopqrstuvwxyz" to 
        *  distinguish between kmalloc and vmalloc initialized memory. 
	*/
    int i;
    for(i=0;i<26;i++){
            vmalloc_ptr[i] = 97 + i;
    }
    return 0;
}

static void mmap_kmalloc_cleanup(void) {

    for (virt_addr=(unsigned long)kmalloc_area; virt_addr < (unsigned long)kmalloc_area + LEN;
            virt_addr+=PAGE_SIZE) {
                    // clear all pages
                    ClearPageReserved(virt_to_page(virt_addr));
    }
    kfree(kmalloc_ptr);
}


static void mmap_vmem_cleanup(void) {

	unsigned long virt_addr;
    for (virt_addr=(unsigned long)vmalloc_ptr; virt_addr < (unsigned long)vmalloc_ptr + LEN;
            virt_addr+=PAGE_SIZE) {
                    // clear all pages
                    ClearPageReserved(vmalloc_to_page((unsigned long *)virt_addr));
    }
    vfree(vmalloc_ptr);
}

static int mmaper_open(struct inode *inode, struct file *file){

    /**
     * Here we use kmalloc when the minor number is 0;
     * we use vmalloc if no. 
     */

    int minor_num = MINOR(inode->i_rdev);
	if(!minor_num) 
        mmap_kmalloc_init();
    else
        mmap_vmem_init();

    // store the device minor number
    *(file->private_data) = minor_num;

	return 0;
}

static int mmaper_release(struct inode *inode, struct file *file){

	if(!MINOR(inode->i_rdev)) 
        mmap_kmalloc_cleanup();
    else
        mmap_vmem_cleanup();

	return 0;
}

static int mmappr_mmap(struct file *file, struct vm_area_struct *vma) {

    if(!*(file->private_data)){
        return mmap_kmalloc(filp,vma);
    }
    else{
        return mmap_vmem(filp,vma);
    }
}

static struct file_operations mmaper_fops = {
	open: 	 mmaper_open,
	release: mmaper_release,
	map:     mmappr_mmap,
	owner:	 THIS_MODULE
};

static int __init mmaper_init(void){

	int result;
	int major;

	/** 
 	  * Dynamically allocate Major Number.  
	  * If you always want the same major number then use MKDEV and 
	  * register_chrdev
	  * dev = MKDEV(DEV_MAJOR, DEV_MINOR);
	  * ret = register_chrdev(dev,HELLOPLUS,&hello_fops)
	  * unregister it on failure by: unregister_chrdev(DEV_MAJOR,HELLOPLUS);
	  */

	result = alloc_chrdev_region(&dev, 0, 2, dev_name);
	if (result<0) 
		return result;

    major = MAJOR(dev);

	printk("The device is registered by Major no: %d\n", major);

	// Allocate a cdev structure 
    mmaper_cdev = cdev_alloc();
	
	// Attach hello fops methods with the cdev: hello_cdev->ops=&hello_fops 
	cdev_init (mmaper_cdev, &mmaper_fops);
    mmaper_cdev->owner = THIS_MODULE;

	// Connect the assigned major number to the cdev 
    result = cdev_add(mmaper_cdev, dev, 1);
    if (result<0){
		printk("Error in registering the module\n");
        unregister_chrdev_region(dev, 1);
        return result;
    }

	printk(KERN_INFO "mmaper: %d\n",__LINE__);

    /*
	* Create an entry (class/directory) in sysfs using:
	* class_create() and device_create()
    * for udev to automatically create a device file when module is 
    * loaded and this init function is called.
    */

    mmaper_class = class_create(THIS_MODULE, dev_name);
	if (IS_ERR(mmaper_class)) {
		printk(KERN_ERR "Error creating hello class.\n");
		result = PTR_ERR(mmaper_class);
		cdev_del(mmaper_cdev);
		unregister_chrdev_region(dev, 1);
		return -1;
    }

    device_create(mmaper_class,NULL,dev,NULL,"mmaper");

	printk(KERN_INFO "mmaper: loading\n");

	return 0;
}


static void __exit mmaper_cleanup(void){

	printk(KERN_INFO "mmaper: unloading...\n");

	cdev_del(mmaper_cdev);
	device_destroy(mmaper_class, dev);
	class_destroy(mmaper_class);
	unregister_chrdev_region(dev,1);
}

module_init(mmaper_init);
module_exit(mmaper_cleanup);












