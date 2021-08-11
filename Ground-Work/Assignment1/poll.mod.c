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
	{ 0x27d03f38, "device_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x19794500, "cdev_del" },
	{ 0x47fa1b1c, "__class_create" },
	{ 0x5ffccd73, "cdev_add" },
	{ 0x807ede20, "cdev_init" },
	{ 0x23ae549f, "cdev_alloc" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x58fefba8, "pv_ops" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xc3aaf0a9, "__put_user_1" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0xcf2a6966, "up" },
	{ 0xc5850110, "printk" },
	{ 0x167e7f9d, "__get_user_1" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x1000e51, "schedule" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x6bd0e573, "down_interruptible" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0F9ABA93F3BB088403FBEF0");
