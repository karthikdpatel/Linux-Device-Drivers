#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");

static dev_t dev_num;
static struct cdev * sleepy_dev_cdev;

static int flag = 0;

static DECLARE_WAIT_QUEUE_HEAD(wq);


static ssize_t sleepy_read(struct file * file_p, char __user * buff, size_t count, loff_t * pos)
{
	printk(KERN_DEBUG "SLEEPY_DRIVER : Process %d(%s) is going to sleep\n", current->pid, current->comm);
	wait_event_interruptible(wq, flag != 0);
	flag = 0;
	printk("SLEEPY_DRIVER : Process %d(%s) is woken up\n", current->pid, current->comm);
	return 0;
}

static ssize_t sleepy_write(struct file * file_p, const char __user * buff, size_t count, loff_t * pos)
{
	printk(KERN_DEBUG "SLEEPY_DRIVER : Process %d(%s) is waking up readers\n", current->pid, current->comm);
	wake_up_interruptible(&wq);
	flag = 1;
	return count;
}


static int sleepy_open(struct inode * inode, struct file * filp)
{
	printk(KERN_DEBUG "SLEEPY_DRIVER : sleepy_driver open\n");
	return 0;
}

static int sleepy_release(struct inode * inode, struct file * filp)
{
	printk(KERN_DEBUG "SLEEPY_DRIVER : sleepy_driver release\n");
	return 0;
}


static int allocate_device_num_dynmaically(void)
{
	int minor = 0, count = 1;
	char * name = "sleepy_driver_char_device";
			
	int res = alloc_chrdev_region(&dev_num, minor, count, name);
	if (res < 0)
	{
		printk("SLEEPY_DRIVER FAILED : Failed to allocate device num, exited with result:%d", res);
		return res;
	}
	printk("SLEEPY_DRIVER : Allocated device with device num:%d, major number:%d", dev_num, MAJOR(dev_num));
	return 0;
}


static int __init scull_init(void)
{
	static struct file_operations scull_fops = {
		.owner = THIS_MODULE,
		.open = sleepy_open,
		.release = sleepy_release,
		.read = sleepy_read,
		.write = sleepy_write,
	};
	
	printk("---------------------\n");
	if(allocate_device_num_dynmaically())
	{
		return -1;
	}
	
	sleepy_dev_cdev = cdev_alloc();
	if (!sleepy_dev_cdev)
	{
		printk("SLEEPY_DRIVER FAILED : Failed to allocate cdev structure\n");
		unregister_chrdev_region(dev_num, 1);
		return -ENOMEM;
	}
	cdev_init(sleepy_dev_cdev, &scull_fops);
	sleepy_dev_cdev->owner = THIS_MODULE;
	
	if (cdev_add(sleepy_dev_cdev, dev_num, 1) < 0) 
	{
		kfree(sleepy_dev_cdev);
        	unregister_chrdev_region(dev_num, 1);
        	printk("SLEEPY_DRIVER FAILED : Failed to add cdev\n");
	        return -1;
        }
        printk("SLEEPY_DRIVER : Added char dev to kernel\n");
	
	printk(KERN_DEBUG "SLEEPY_DRIVER : Init sleepy_driver\n");
	return 0;
}

static void __exit scull_exit(void)
{
	printk(KERN_DEBUG "SLEEPY_DRIVER : Exit sleepy_driver\n");
}

module_init(scull_init);
module_exit(scull_exit);

