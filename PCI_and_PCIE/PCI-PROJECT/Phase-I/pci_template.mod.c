#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xc6c01fa, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x7d0571e5, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0x4203c482, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x62e4100, __VMLINUX_SYMBOL_STR(register_netdev) },
	{ 0xf10de535, __VMLINUX_SYMBOL_STR(ioread8) },
	{ 0x363cd1b7, __VMLINUX_SYMBOL_STR(alloc_etherdev_mqs) },
	{ 0x2b9e8aac, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0xbbd78bd4, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0x1fedf0f4, __VMLINUX_SYMBOL_STR(__request_region) },
	{ 0x69a358a6, __VMLINUX_SYMBOL_STR(iomem_resource) },
	{ 0x48254905, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0xf15e6c6b, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xb19de74, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x81fcd7c8, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x92a94ad2, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x420ffece, __VMLINUX_SYMBOL_STR(consume_skb) },
	{ 0xd17fbaf3, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x323a933d, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0x7a683160, __VMLINUX_SYMBOL_STR(pci_iounmap) },
	{ 0x11ec5e3, __VMLINUX_SYMBOL_STR(free_netdev) },
	{ 0x7cbab064, __VMLINUX_SYMBOL_STR(unregister_netdev) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010ECd00008139sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "C01073AF44E5E9E76D14A81");
