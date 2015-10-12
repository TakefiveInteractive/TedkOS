#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xfc4fa38d, "struct_module" },
	{ 0x8e810e6c, "kmalloc_caches" },
	{ 0xa120d33c, "tty_unregister_ldisc" },
	{ 0x1b7d4074, "printk" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0x9079fc1f, "kmem_cache_alloc" },
	{ 0x37a0cba, "kfree" },
	{ 0x9c976d3a, "tty_register_ldisc" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "BBE623E15619D1E50FBA9B6");
