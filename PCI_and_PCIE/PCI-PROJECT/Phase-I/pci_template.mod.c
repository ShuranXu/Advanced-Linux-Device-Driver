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
	{ 0x3b67ff49, "pci_unregister_driver" },
	{ 0xfbfddd92, "__pci_register_driver" },
	{ 0x885de096, "_dev_err" },
	{ 0x1b29e540, "register_netdev" },
	{ 0xf10de535, "ioread8" },
	{ 0x6f2335ff, "alloc_etherdev_mqs" },
	{ 0x6d546ce, "dma_set_coherent_mask" },
	{ 0x3f115012, "dma_set_mask" },
	{ 0x93a219c, "ioremap_nocache" },
	{ 0x85bd1608, "__request_region" },
	{ 0x77358855, "iomem_resource" },
	{ 0x97b51df7, "pci_set_master" },
	{ 0x54836bf2, "pci_enable_device" },
	{ 0xb3a666c0, "kmem_cache_alloc_trace" },
	{ 0x31ca542f, "kmalloc_caches" },
	{ 0x9dd6dcb, "consume_skb" },
	{ 0xf38a3177, "pci_disable_device" },
	{ 0x37a0cba, "kfree" },
	{ 0x75cf665f, "pci_release_regions" },
	{ 0x47a1aa13, "pci_iounmap" },
	{ 0x2377e92e, "free_netdev" },
	{ 0x45b388de, "unregister_netdev" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v000010ECd00008139sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "67515263B1EAB2765CA5721");
