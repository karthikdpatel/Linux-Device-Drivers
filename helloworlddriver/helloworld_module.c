#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

static int num = 1;

module_param(num, int, S_IRUGO);

static int hello_init(void)
{
	printk(KERN_ALERT "Hello World1 = %d", num);
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Exiting Hello world1");
}

module_init(hello_init);
module_exit(hello_exit);
