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
	{ 0x1b29e540, "register_netdev" },
	{ 0x6f2335ff, "alloc_etherdev_mqs" },
	{ 0x885de096, "_dev_err" },
	{ 0xb3a666c0, "kmem_cache_alloc_trace" },
	{ 0x31ca542f, "kmalloc_caches" },
	{ 0x976ee327, "__netdev_alloc_skb" },
	{ 0xb4639287, "netif_rx" },
	{ 0xb9f28860, "eth_type_trans" },
	{ 0xe579f4bb, "skb_put" },
	{ 0xe43b73d4, "netif_carrier_off" },
	{ 0xc48429db, "netif_carrier_on" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x27e5949e, "usb_unlink_urb" },
	{ 0x90777d9a, "usb_control_msg" },
	{ 0x9f84fca9, "__dev_kfree_skb_irq" },
	{ 0x7e24b7cb, "usb_submit_urb" },
	{ 0x85eae14b, "usb_alloc_urb" },
	{ 0x2377e92e, "free_netdev" },
	{ 0x45b388de, "unregister_netdev" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v0BDAp8150d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "6A0937E00813E5D36F94570");
