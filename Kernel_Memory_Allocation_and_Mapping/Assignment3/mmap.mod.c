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
	{ 0x999e8297, "vfree" },
	{ 0x438ecfcf, "remove_proc_entry" },
	{ 0xcb91c2e0, "class_destroy" },
	{ 0xda5087c1, "vmalloc_to_page" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0xb8b9f817, "kmalloc_order_trace" },
	{ 0x19e0af64, "device_destroy" },
	{ 0xf39c273c, "proc_create" },
	{ 0x27d03f38, "device_create" },
	{ 0x19794500, "cdev_del" },
	{ 0x47fa1b1c, "__class_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x5ffccd73, "cdev_add" },
	{ 0x807ede20, "cdev_init" },
	{ 0x23ae549f, "cdev_alloc" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x4c9d28b0, "phys_base" },
	{ 0xc5850110, "printk" },
	{ 0x3744cf36, "vmalloc_to_pfn" },
	{ 0x6aa01b2f, "remap_pfn_range" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x3b29a28b, "kmem_cache_alloc_trace" },
	{ 0x8b1f4bcb, "kmalloc_caches" },
	{ 0x37a0cba, "kfree" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B017D0C132AEEA1579C3728");
