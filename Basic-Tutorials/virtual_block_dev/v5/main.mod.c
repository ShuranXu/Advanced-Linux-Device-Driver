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
	{ 0x7f89e2e9, "blk_cleanup_queue" },
	{ 0x329ae093, "put_disk" },
	{ 0xa2232a30, "del_gendisk" },
	{ 0x6d4af13f, "device_add_disk" },
	{ 0xe8f9ec41, "set_capacity" },
	{ 0xa9f0103a, "__alloc_disk_node" },
	{ 0x37a0cba, "kfree" },
	{ 0x21b2282f, "blk_mq_init_sq_queue" },
	{ 0xb8b9f817, "kmalloc_order_trace" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0x3b29a28b, "kmem_cache_alloc_trace" },
	{ 0x8b1f4bcb, "kmalloc_caches" },
	{ 0x720a27a7, "__register_blkdev" },
	{ 0xf80edb93, "__blk_mq_end_request" },
	{ 0x78ad7aea, "blk_update_request" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x2139b17e, "blk_mq_start_request" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "06105FA81B2483E1EB40318");
