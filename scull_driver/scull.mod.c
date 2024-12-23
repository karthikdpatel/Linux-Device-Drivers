#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

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
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x178f4a8a, "cdev_alloc" },
	{ 0x5c658724, "cdev_init" },
	{ 0xa2dc7695, "cdev_add" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xd58dd2b1, "cdev_del" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x122c3a7e, "_printk" },
	{ 0x37a0cba, "kfree" },
	{ 0x4c03a563, "random_kmalloc_seed" },
	{ 0xa63b4eed, "kmalloc_caches" },
	{ 0x59ffeca6, "kmalloc_trace" },
	{ 0x8d6aff89, "__put_user_nocheck_4" },
	{ 0xf079b8f9, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "A4D5FFEE7DB84E07580A6D0");
