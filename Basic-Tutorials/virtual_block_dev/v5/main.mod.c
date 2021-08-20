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
	{ 0x245b8564, "blk_cleanup_queue" },
	{ 0x5fb50da5, "put_disk" },
	{ 0xa5b35e62, "del_gendisk" },
	{ 0x27e0a3ff, "device_add_disk" },
	{ 0x1b81e402, "__alloc_disk_node" },
	{ 0x37a0cba, "kfree" },
	{ 0xe0fe5f5d, "blk_mq_init_sq_queue" },
	{ 0xb8b9f817, "kmalloc_order_trace" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0xb3a666c0, "kmem_cache_alloc_trace" },
	{ 0x31ca542f, "kmalloc_caches" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0xd4b07586, "__blk_mq_end_request" },
	{ 0xe6a14f29, "blk_update_request" },
	{ 0x56470118, "__warn_printk" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x1ffad3a4, "blk_mq_start_request" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "06105FA81B2483E1EB40318");
