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
	{ 0xba978e76, "usb_deregister" },
	{ 0x9528195c, "usb_register_driver" },
	{ 0x885de096, "_dev_err" },
	{ 0x90777d9a, "usb_control_msg" },
	{ 0x1b29e540, "register_netdev" },
	{ 0x6f2335ff, "alloc_etherdev_mqs" },
	{ 0x9dd6dcb, "consume_skb" },
	{ 0x2377e92e, "free_netdev" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v0BDAp8150d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "3CB717149071305A1C8A2CF");
