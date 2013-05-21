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
__used
__attribute__((section("__versions"))) = {
	{ 0x3ad992a9, "module_layout" },
	{ 0x39cb1255, "kmalloc_caches" },
	{ 0xf9a482f9, "msleep" },
	{ 0x1b9981cc, "set_irq_wake" },
	{ 0x2897818a, "i2c_smbus_read_byte_data" },
	{ 0x27bbf221, "disable_irq_nosync" },
	{ 0xd9a06a0f, "i2c_del_driver" },
	{ 0x53720c36, "i2c_smbus_write_byte_data" },
	{ 0x27845b78, "i2c_transfer" },
	{ 0xc633495b, "schedule_work" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x5232263e, "input_event" },
	{ 0x5f754e5a, "memset" },
	{ 0xea147363, "printk" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0x43b0c9c3, "preempt_schedule" },
	{ 0x2f0323e1, "i2c_register_driver" },
	{ 0xf1b71002, "kmem_cache_alloc" },
	{ 0x1348f54b, "dev_driver_string" },
	{ 0xfcec0987, "enable_irq" },
	{ 0x37a0cba, "kfree" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("i2c:sii8334_PAGE_TPI");
MODULE_ALIAS("i2c:sii8334_PAGE_TX_L0");
MODULE_ALIAS("i2c:sii8334_PAGE_TX_L1");
MODULE_ALIAS("i2c:sii8334_PAGE_TX_2");
MODULE_ALIAS("i2c:sii8334_PAGE_TX_3");
MODULE_ALIAS("i2c:sii8334_PAGE_CBUS");

MODULE_INFO(srcversion, "07BD7780A3FB0167D2AC67B");
