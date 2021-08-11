#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0x243c757b, "module_layout" },
	{ 0xcb91c2e0, "class_destroy" },
	{ 0x19e0af64, "device_destroy" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0x27d03f38, "device_create" },
	{ 0x19794500, "cdev_del" },
	{ 0x47fa1b1c, "__class_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x5ffccd73, "cdev_add" },
	{ 0x807ede20, "cdev_init" },
	{ 0x23ae549f, "cdev_alloc" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xd102a031, "vc_cons" },
	{ 0x4e6e8ea7, "fg_console" },
	{ 0x24d273d1, "add_timer" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7A7D7D56A710A2BF0E50CA9");
