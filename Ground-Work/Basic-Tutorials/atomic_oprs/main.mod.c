#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xa4b86400, "module_layout" },
	{ 0xf6cb2bfb, "device_destroy" },
	{ 0xa5cc5d9f, "kthread_stop" },
	{ 0x725a8218, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x74007fb7, "class_destroy" },
	{ 0xe4b38bdc, "wake_up_process" },
	{ 0xe8bc695c, "kthread_create_on_node" },
	{ 0x8b58c8f8, "device_create" },
	{ 0xcaed4f8, "__class_create" },
	{ 0xd2198226, "cdev_add" },
	{ 0xb5739c63, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xf9a482f9, "msleep" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "1588A03F237621B2B7C4111");
