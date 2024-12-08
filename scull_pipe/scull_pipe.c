#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "scull_pipe.h"
#include <linux/semaphore.h>

MODULE_LICENSE("Dual BSD/GPL");

static dev_t dev_num;
static struct cdev * sleepy_dev_cdev;

static ssize_t scull_pipe_read(struct file * file_p, char __user * buff, size_t count, loff_t * pos)
{

	printk(KERN_DEBUG "SCULL_PIPE : SCULL_PIPE read\n");
	struct scull_pipe_dev * scull_pipe_dev = file_p->private_data;
	
	if(down_interruptible(&scull_pipe_dev->sem))
	{
		return -ERESTARTSYS;
	}
	
	while(scull_pipe_dev->rp == scull_pipe_dev->wp)
	{
		up(&scull_pipe_dev->sem);
		
		/*if(file_p->f_flags && O_NONBLOCK)
		{
			return -EAGAIN;
		}*/
		
		printk(KERN_DEBUG "SCULL_PIPE : scull_pipe going to sleep no data to read, process:%s\n", current->comm);
		if(wait_event_interruptible(scull_pipe_dev->inq, scull_pipe_dev->rp != scull_pipe_dev->wp))
		{
			return -ERESTARTSYS;
		}
		
		if(down_interruptible(&scull_pipe_dev->sem))
		{
			return -ERESTARTSYS;
		}
		
	}
	
	if(scull_pipe_dev->rp < scull_pipe_dev->wp)
	{
		count = min_t(unsigned int, scull_pipe_dev->wp - scull_pipe_dev->rp, count);
	}
	else
	{
		count = min_t(unsigned int, scull_pipe_dev->buffer_end - scull_pipe_dev->rp, count);
	}
	
	if(copy_to_user(buff, scull_pipe_dev->rp, count))
	{
		up(&scull_pipe_dev->sem);
		return -EFAULT;
	}
	
	scull_pipe_dev->rp += count;
	if(scull_pipe_dev->rp == scull_pipe_dev->buffer_end)
	{
		scull_pipe_dev->rp = scull_pipe_dev->buffer_start;
	}
	(* pos) += count;
	
	up(&scull_pipe_dev->sem);
	
	wake_up_interruptible(&scull_pipe_dev->outq);
	
	return count;
}

static ssize_t scull_pipe_write(struct file * file_p, const char __user * buff, size_t count, loff_t * pos)
{
	printk(KERN_DEBUG "SCULL_PIPE : SCULL_PIPE write\n");
	struct scull_pipe_dev * scull_pipe_dev = file_p->private_data;
	
	if(down_interruptible(&scull_pipe_dev->sem))
	{
		return -ERESTARTSYS;
	}
	
	while( (scull_pipe_dev->rp == scull_pipe_dev->wp - 1) || (scull_pipe_dev->rp == scull_pipe_dev->buffer_start && scull_pipe_dev->wp == scull_pipe_dev->buffer_end) )
	{
		up(&scull_pipe_dev->sem);
		
		/*if(file_p->f_flags & O_NONBLOCK)
		{
			return -EAGAIN;
		}*/
		
		printk(KERN_DEBUG "SCULL_PIPE : scull_pipe going to sleep no space left to write, process:%s\n", current->comm);
		if(wait_event_interruptible(scull_pipe_dev->outq, (scull_pipe_dev->rp != scull_pipe_dev->wp - 1) && (scull_pipe_dev->rp != scull_pipe_dev->buffer_start || scull_pipe_dev->wp != scull_pipe_dev->buffer_end)))
		{
			return -ERESTARTSYS;
		}
		
		if(down_interruptible(&scull_pipe_dev->sem))
		{
			return -ERESTARTSYS;
		}
		
	}
	
	if(scull_pipe_dev->rp <= scull_pipe_dev->wp)
	{
		count = min_t(unsigned int, count, scull_pipe_dev->buffer_end - scull_pipe_dev->wp);
	}
	else
	{
		count = min_t(unsigned int, count, scull_pipe_dev->rp - scull_pipe_dev->wp);
	}
	
	if(copy_from_user(scull_pipe_dev->wp, buff, count))
	{
		up(&scull_pipe_dev->sem);
		
		return -EFAULT;
	}
	
	scull_pipe_dev->wp += count;
	
	if(scull_pipe_dev->wp == scull_pipe_dev->buffer_end)
	{
		scull_pipe_dev->wp = scull_pipe_dev->buffer_start;
	}
	
	up(&scull_pipe_dev->sem);
	
	wake_up_interruptible(&scull_pipe_dev->inq);
	
	return count;
}

static int scull_pipe_open(struct inode * inode, struct file * filp)
{
	printk(KERN_DEBUG "SCULL_PIPE : SCULL_PIPE open\n");
	
	struct scull_pipe_dev * scull_pipe_dev;
	scull_pipe_dev = container_of(inode->i_cdev, struct scull_pipe_dev, cdev);
	
	if( scull_pipe_dev == NULL || scull_pipe_dev->buffer_start == NULL || scull_pipe_dev->buffer_end == NULL || scull_pipe_dev->buffer_start + scull_pipe_dev->BUFFER_SIZE != scull_pipe_dev->buffer_end)
	{
		printk(KERN_DEBUG "SCULL_PIPE : creating buffer space\n");
		scull_pipe_dev->BUFFER_SIZE = 10;
		scull_pipe_dev->buffer_start = kmalloc(scull_pipe_dev->BUFFER_SIZE, GFP_KERNEL);
		scull_pipe_dev->buffer_end = scull_pipe_dev->buffer_start + scull_pipe_dev->BUFFER_SIZE;
		scull_pipe_dev->rp = scull_pipe_dev->buffer_start;
		scull_pipe_dev->wp = scull_pipe_dev->buffer_start;
		
		init_waitqueue_head(&scull_pipe_dev->inq);
		init_waitqueue_head(&scull_pipe_dev->outq);
		
		sema_init(&scull_pipe_dev->sem, 1);
	}
	
	filp->private_data = scull_pipe_dev;
	return 0;
}

static int scull_pipe_release(struct inode * inode, struct file * filp)
{
	printk(KERN_DEBUG "SCULL_PIPE : SCULL_PIPE release\n");
	return 0;
}

static int allocate_device_num_dynmaically(void)
{
	int minor = 0, count = 1;
	char * name = "scull_pipe_driver";
			
	int res = alloc_chrdev_region(&dev_num, minor, count, name);
	if (res < 0)
	{
		printk("SCULL_PIPE FAILED : Failed to allocate device num, exited with result:%d", res);
		return res;
	}
	printk("SCULL_PIPE : Allocated device with device num:%d, major number:%d", dev_num, MAJOR(dev_num));
	return 0;
}

static int __init scull_init(void)
{
	static struct file_operations scull_pipe_fops = {
		.owner = THIS_MODULE,
		.open = scull_pipe_open,
		.release = scull_pipe_release,
		.read = scull_pipe_read,
		.write = scull_pipe_write,
	};
	
	printk("---------------------\n");
	if(allocate_device_num_dynmaically())
	{
		return -1;
	}
	
	sleepy_dev_cdev = cdev_alloc();
	if (!sleepy_dev_cdev)
	{
		printk("SCULL PIPE FAILED : Failed to allocate cdev structure\n");
		unregister_chrdev_region(dev_num, 1);
		return -ENOMEM;
	}
	cdev_init(sleepy_dev_cdev, &scull_pipe_fops);
	sleepy_dev_cdev->owner = THIS_MODULE;
	
	if (cdev_add(sleepy_dev_cdev, dev_num, 1) < 0) 
	{
		kfree(sleepy_dev_cdev);
        	unregister_chrdev_region(dev_num, 1);
        	printk("SCULL PIPE FAILED : Failed to add cdev\n");
	        return -1;
        }
        printk("SCULL PIPE : Added char dev to kernel\n");
	
	printk(KERN_DEBUG "SCULL_PIPE : Init SCULL_PIPE\n");
	return 0;
}

static void __exit scull_exit(void)
{
	cdev_del(sleepy_dev_cdev);
	printk("SCULL PIPE : Char dev deleted from kernel\n");
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_DEBUG "SCULL PIPE : Exit SCULL_PIPE\n");
}

module_init(scull_init);
module_exit(scull_exit);

