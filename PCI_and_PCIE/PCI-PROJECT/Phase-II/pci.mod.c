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
	{ 0x1b29e540, "register_netdev" },
	{ 0x6f2335ff, "alloc_etherdev_mqs" },
	{ 0x6d546ce, "dma_set_coherent_mask" },
	{ 0x3f115012, "dma_set_mask" },
	{ 0x93a219c, "ioremap_nocache" },
	{ 0x85bd1608, "__request_region" },
	{ 0x77358855, "iomem_resource" },
	{ 0x97b51df7, "pci_set_master" },
	{ 0xc22ab493, "pci_set_mwi" },
	{ 0x54836bf2, "pci_enable_device" },
	{ 0x9dd6dcb, "consume_skb" },
	{ 0x280f3762, "skb_copy_and_csum_dev" },
	{ 0xb4639287, "netif_rx" },
	{ 0xb9f28860, "eth_type_trans" },
	{ 0xe579f4bb, "skb_put" },
	{ 0x976ee327, "__netdev_alloc_skb" },
	{ 0x4c0e404a, "netif_tx_wake_queue" },
	{ 0xe484e35f, "ioread32" },
	{ 0x4a453f53, "iowrite32" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x885de096, "_dev_err" },
	{ 0xae45846e, "dma_alloc_attrs" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x1f309b21, "dma_free_attrs" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xc5534d64, "ioread16" },
	{ 0x6a86bc1, "iowrite16" },
	{ 0xf10de535, "ioread8" },
	{ 0x848d372e, "iowrite8" },
	{ 0xf38a3177, "pci_disable_device" },
	{ 0x210b2403, "pci_clear_mwi" },
	{ 0x75cf665f, "pci_release_regions" },
	{ 0x47a1aa13, "pci_iounmap" },
	{ 0x2377e92e, "free_netdev" },
	{ 0x45b388de, "unregister_netdev" },
	{ 0x3812050a, "_raw_spin_unlock_irqrestore" },
	{ 0x51760917, "_raw_spin_lock_irqsave" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v000010ECd00008139sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "C54E8AC967AC5DBF84865BB");
