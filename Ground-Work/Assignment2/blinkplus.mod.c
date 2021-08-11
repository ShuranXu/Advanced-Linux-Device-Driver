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
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xd102a031, "vc_cons" },
	{ 0x4e6e8ea7, "fg_console" },
	{ 0x24d273d1, "add_timer" },
	{ 0xc5850110, "printk" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "AD8EE0E14F2AE20C7724998");
