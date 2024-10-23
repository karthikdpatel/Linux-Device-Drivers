	#include <linux/init.h>
	#include <linux/module.h>
	#include <linux/fs.h>
	#include <linux/cdev.h>
	#include "scull.h"
	#include <linux/slab.h>

	dev_t dev_num;
	struct cdev *chr_dev;
	#define SCULL_SIZE 1024
	static char SCULL_BUFFER[SCULL_SIZE];

	MODULE_LICENSE("Dual BSD/GPL");

	static int allocate_device_num_dynmaically(void)
	{
		int minor = 0, count = 1;
		char * name = "scull_char_device";
		
		
		int res = alloc_chrdev_region(&dev_num, minor, count, name);
		if (res < 0)
		{
			//printk(KERN_ALERT "Failed to allocate device num, exited with result:%d", res);
			return res;
		}
		//printk(KERN_ALERT "Allocated device with device num:%d", dev_num);
		return 0;
	}

	static struct scull_qset * get_scull_item(struct scull_dev * dev, int qset_item_pos)
	{
		struct scull_qset * set = dev->data;
		int i = 0;
		
		while(i < qset_item_pos)
		{
			set = set->next;
			i += 1;
		}
		
		return set;
	}

	static ssize_t scull_read(struct file * file_p, char __user * buff, size_t count, loff_t * pos)
	{

		printk("SCULL_READ_STARTED\n");
		
		if(*pos >= SCULL_SIZE)
		{
			return 0;
		}
		
		if( *pos + count > SCULL_SIZE)
		{
			count = SCULL_SIZE - *pos;
		}
		
		if(copy_to_user(buff, SCULL_BUFFER + (*pos), count))
		{
			return -1;
		}
		
		(*pos) += count;
		
		/*struct scull_dev *dev = file_p->private_data;
		
		int qset_size = dev->quantum * dev->qset;
		int qset_item_pos = (*pos) / qset_size;
		
		if(*pos > dev->size)
		{
			return -1;
		}
		
		if(*pos + count > dev->size)
		{
			count = dev->size - (*pos);
		}
		
		int remaining = (*pos) % qset_size;
		int buff_pos = remaining / dev->quantum;
		int quant_pos = remaining % dev->quantum;
		
		struct scull_qset * qset_item = get_scull_item(dev, qset_item_pos);
		
		if(!qset_item || !qset_item->data || !qset_item->data[buff_pos])
		{
			return -1;
		}
		
		if(count > dev->quantum - quant_pos)
		{
			count = dev->quantum - quant_pos;
		}
		
		if(copy_to_user(buff, qset_item->data[buff_pos] + quant_pos, count))
		{
			return -1;
		}
		
		(*pos) += count;
		*/
		printk("SCULL_READ_ENDED\n");
		
		return count;
	}

	static ssize_t scull_write(struct file * file_p, const char __user * buff, size_t count, loff_t * pos)
	{
		printk("SCULL_WRITE_STARTED\n");
		
		if(*pos >= SCULL_SIZE)
		{
			return 0;
		}
		
		if(*pos + count > SCULL_SIZE)
		{
			count = SCULL_SIZE - (*pos);
		}
		
		if(copy_from_user(SCULL_BUFFER + (*pos), buff, count))
		{
			return -1;
		}
		
		*pos += count;
		
		
		/*struct scull_dev *dev = file_p->private_data;
		
		int qset_size = dev->quantum * dev->qset;
		int qset_item_pos = (*pos) / qset_size;
		
		
		int remaining = (*pos) % qset_size;
		int buff_pos = remaining / dev->quantum;
		int quant_pos = remaining % dev->quantum;
		
		struct scull_qset * qset_item = get_scull_item(dev, qset_item_pos);
		
		if(!qset_item)
		{
			return -1;
		}
		
		if(!qset_item->data)
		{
			qset_item->data = kmalloc(sizeof(void *) * dev->qset, GFP_KERNEL);
		}
		
		if(!qset_item->data[buff_pos])
		{
			qset_item->data[buff_pos] = kmalloc(dev->quantum, GFP_KERNEL);
		}
		
		if(count > dev->quantum - quant_pos)
		{
			count = (dev->quantum - quant_pos);
		}
		
		
		if(copy_from_user(qset_item->data[buff_pos] + quant_pos, buff, count))
		{
			return -1;
		}
		
		(*pos) += count;
		
		if(dev->size < (*pos))
		{
			dev->size = (*pos);
		}
		*/
		
		printk("SCULL_WRITE_ENDED\n");
		
		return count;
		
		
	}

	static void scull_trim(struct scull_dev * dev)
	{
		struct scull_qset * qset_ptr = dev->data;
		struct scull_qset * next;
		int i;
		
		while(qset_ptr)
		{
			for(i=0; i < dev->qset ; i++)
			{
				kfree(qset_ptr->data[i]);
			}
			kfree(qset_ptr->data);
			next = qset_ptr->next;
			kfree(qset_ptr);
			qset_ptr = next;
		}
		
		dev->data = NULL;
		dev->quantum = SCULL_QUANTUM;
		dev->qset = SCULL_QSET;
		dev->size = 0;
	}

	static int scull_open(struct inode * inode, struct file * filp)
	{
		printk("SUCCESS_SCULL : scull_open started\n");
		/*struct scull_dev *chr_dev;
		chr_dev = container_of(inode->i_cdev, struct scull_dev, cdev);
		filp->private_data = chr_dev;
		scull_trim(chr_dev);
		*/
		printk("SUCCESS_SCULL : scull_open ended\n");
		return 0;
	}

	static int scull_release(struct inode * inode, struct file * filp)
	{
		printk("SUCCESS_SCULL : scull_release()\n");
		return 0;
	}


	static int scull_setup(void)
	{
		int res = allocate_device_num_dynmaically();
		if (res < 0) {
			printk("ERROR_SCULL : Failed to allocate device number\n");
			return res;
		}
		printk("SUCCESS_SCULL : Allocated device with device num:%d, major number:%d\n", dev_num, MAJOR(dev_num));
		
		struct file_operations fops = {
			.owner = THIS_MODULE,
			.open = scull_open,
			.release = scull_release,
			.read = scull_read,
			.write = scull_write,
		};
		
		chr_dev = cdev_alloc();
		if (!chr_dev)
		{
			printk("ERROR_SCULL : Failed to allocate cdev structure\n");
			unregister_chrdev_region(dev_num, 1);
			return -ENOMEM;
		}
		cdev_init(chr_dev, &fops);
		chr_dev->owner = THIS_MODULE;

		
		res = cdev_add(chr_dev, dev_num, 1);
		
		if(res < 0)
		{
			printk("ERROR_SCULL: Error adding cdev to kernel\n");
			kfree(chr_dev);
			unregister_chrdev_region(dev_num, 1);
			return res;
		}
		
		printk("SUCCESS_SCULL : Added char dev to kernel\n");
		
		return 0;
	}


	static int hello_init(void)
	{

		printk("START_SCULL : -------------");
		int res = scull_setup();
		
		if (res < 0) {
			printk("ERROR_SCULL : Failed to setup device\n");
			return res;
		}
		printk("START_SCULL: Module initialized successfully\n");
		return res;
		
	}

	static void hello_exit(void)
	{
		int major = 50, count = 1;
		//char * name = "mydevice";
		
		cdev_del(chr_dev);
		printk("SUCCESS_SCULL : Char dev deleted from kernel\n");
		unregister_chrdev_region(dev_num, count);
		printk("SUCCESS_SCULL : Deallocated device with device num:%d, major number:%d\n", dev_num, MAJOR(dev_num));
	}

	module_init(hello_init);
	module_exit(hello_exit);
