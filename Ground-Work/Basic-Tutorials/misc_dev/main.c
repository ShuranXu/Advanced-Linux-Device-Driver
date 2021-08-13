/***************************************************************************//**
*  \file       misc_driver.c
*
*  \details    Simple misc driver explanation
*
*  \author     EmbeTronicX
*
*  \Tested with kernel 5.3.0-42-generic
*
* Tutorial Link: https://embetronicx.com/tutorials/linux/device-drivers/misc-device-driver/
*
*******************************************************************************/

/**
 * @file main.c
 * 
 * @author Shuran Xu
 * @brief 
 * @version 0.1
 * @date 2021-08-13
 * 
 * @copyright Copyright (c) 2021
 * 
 * Description about Misc device:
 * 
 * Misc driver is the miscellaneous driver for miscellaneous devices. We can say that misc 
 * drivers are special and simple character drivers. You can write this misc driver when 
 * you cannot classify your peripheral. This means, if you don’t want to use the major number, 
 * then you can write this misc driver. And also if you want to write a simple driver, 
 * then you can choose a misc driver instead of choosing a character driver.
 * 
 * Difference between character driver and misc driver:
 * 
 * 1. In misc driver, the major number will be 10 and the minor number is user convenient. Whereas, 
 * in character drivers, the user can select their own major and minor number if it is available.
 * 
 * 2. The device node or device file will be automatically generated in misc drivers. Whereas, in 
 * character drivers, the user has to create the device node or device file using cdev_init, 
 * cdev_add, class_create, and device_create.
 * 
 */

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

/*
** This function will be called when we open the Misc device file
*/
static int etx_misc_open(struct inode *inode, struct file *file)
{
    pr_info("EtX misc device open\n");
    return 0;
}

/*
** This function will be called when we close the Misc Device file
*/
static int etx_misc_close(struct inode *inodep, struct file *filp)
{
    pr_info("EtX misc device close\n");
    return 0;
}

/*
** This function will be called when we write the Misc Device file
*/
static ssize_t etx_misc_write(struct file *file, const char __user *buf,
               size_t len, loff_t *ppos)
{
    pr_info("EtX misc device write\n");
    
    /* We are not doing anything with this data now */
    
    return len; 
}
 
/*
** This function will be called when we read the Misc Device file
*/
static ssize_t etx_misc_read(struct file *filp, char __user *buf,
                    size_t count, loff_t *f_pos)
{
    pr_info("EtX misc device read\n");
 
    return 0;
}

//File operation structure 
static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .write          = etx_misc_write,
    .read           = etx_misc_read,
    .open           = etx_misc_open,
    .release        = etx_misc_close,
    .llseek         = no_llseek,
};

//Misc device structure
struct miscdevice etx_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "simple_etx_misc",
    .fops = &fops,
};

/*
** Misc Init function
*/
static int __init misc_init(void)
{
    int error;
 
    error = misc_register(&etx_misc_device);
    if (error) {
        pr_err("misc_register failed!!!\n");
        return error;
    }
 
    pr_info("misc_register init done!!!\n");
    return 0;
}

/*
** Misc exit function
*/
static void __exit misc_exit(void)
{
    misc_deregister(&etx_misc_device);
    pr_info("misc_register exit done!!!\n");
}
 
module_init(misc_init)
module_exit(misc_exit)
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shuran Xu");
MODULE_DESCRIPTION("A simple device driver - Misc Driver");
MODULE_VERSION("1.29");