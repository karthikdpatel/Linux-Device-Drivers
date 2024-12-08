#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "scull.h"
#include <linux/slab.h>
#include "scull_ioctl.h"
#include <asm/uaccess.h>
#include <linux/workqueue.h>


MODULE_LICENSE("Dual BSD/GPL");
static int scull_release(struct inode * inode, struct file * filp);
static int scull_open(struct inode * inode, struct file * filp);
static ssize_t scull_write(struct file * file_p, const char __user * buff, size_t count, loff_t * pos);
static ssize_t scull_read(struct file * file_p, char __user * buff, size_t count, loff_t * pos);
static long scull_ioctl(struct file * filep, unsigned int cmd, unsigned long arg);

void interupt_handler(void *data);

static dev_t dev_num;
static struct cdev * scull_cdev;
static struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.open = scull_open,
	.release = scull_release,
	.read = scull_read,
	.write = scull_write,
	.unlocked_ioctl = scull_ioctl,
};

static struct workqueue_struct *my_wq;
static struct work_struct *work;

struct kmem_cache *scull_quantum_cache;

static long scull_ioctl(struct file * file_p, unsigned int cmd, unsigned long arg)
{
	struct scull_dev * scull_device = file_p->private_data;
	
	int retval = 0, err;
	
	if(_IOC_TYPE(cmd) != SCULL_IOC_MAGIC)
	{
		return -1;
	}
	
	switch(cmd)
	{
		case SCULL_IO_RESET:
			scull_device->quantum = SCULL_QUANTUM;
			scull_device->qset = SCULL_QSET;
			break;
			
		case SCULL_IO_GET_QUANTUM:
		
			if(!access_ok( (int __user *)arg, sizeof(scull_device->quantum)))
			{
				printk("SCULL ERROR : SCULL_IO_SET_QUANTUM access_ok()\n");
				return -1;
			}
		
			if(err=__put_user( scull_device->quantum, (int __user *)arg))
			{
				printk("SCULL ERROR : SCULL_IO_GET_QSET %d\n", err);
				return -1;
			}
		
			/*if(retval = copy_to_user((int __user *)arg, &scull_device->quantum, sizeof(scull_device->quantum)))
			{
				printk("SCULL ERROR : SCULL_IO_GET_QUANTUM %d\n", retval);
				return -1;
			}*/
			break;
			
		case SCULL_IO_GET_QSET:
		
			if(!access_ok( (int __user *)arg, sizeof(scull_device->qset)))
			{
				printk("SCULL ERROR : SCULL_IO_GET_QSET access_ok()\n");
				return -1;
			}
			
			if(err=__put_user( scull_device->qset, (int __user *)arg))
			{
				printk("SCULL ERROR : SCULL_IO_GET_QSET %d\n", err);
				return -1;
			}
		
			/*if(retval = copy_to_user((int __user *)arg, &scull_device->qset, sizeof(scull_device->qset)))
			{
				printk("SCULL ERROR : SCULL_IO_GET_QSET %d\n", retval);
				return -1;
			}*/
			break;
			
		case SCULL_IO_SET_QUANTUM:
		
			
		
			if(copy_from_user(&scull_device->quantum, (int __user *)arg, sizeof(scull_device->quantum)))
			{
				return -1;
			}
			printk("SCULL : New QUANTUM : %d\n", scull_device->quantum);
			break;
			
		case SCULL_IO_SET_QSET:
			if(copy_from_user(&scull_device->qset, (int __user *)arg, sizeof(scull_device->qset)))
			{
				return -1;
			}
			printk("SCULL : New QSET : %d\n", scull_device->qset);
			break;
			
		case SCULL_IO_TELL_QUANTUM:
			printk("SCULL DBG : arg:%ld", arg);
			scull_device->quantum = arg;
			printk("SCULL : New QUANTUM : %d\n", scull_device->quantum);
			break;
			
		case SCULL_IO_TELL_QSET:
			scull_device->qset = arg;
			printk("SCULL : New QSET : %d\n", scull_device->qset);
			break;
			
		case SCULL_IO_QUERY_QUANTUM:
			retval = scull_device->quantum;
			break;
			
		case SCULL_IO_QUERY_QSET:
			retval = scull_device->qset;
			break;
			
		default:
			return -ENOTTY;
			
	}
	
	return retval;
}


static int allocate_device_num_dynmaically(void)
{
	int minor = 0, count = 1;
	char * name = "scull_char_device";
			
	int res = alloc_chrdev_region(&dev_num, minor, count, name);
	if (res < 0)
	{
		printk("SCULL FAILED : Failed to allocate device num, exited with result:%d", res);
		return res;
	}
	printk("SCULL : Allocated device with device num:%d, major number:%d", dev_num, MAJOR(dev_num));
	return 0;
}

static struct scull_qset * get_scull_item(struct scull_dev * scull_device, int scull_qset_item_num)
{
	struct scull_qset *prev = NULL, * qset_ptr = scull_device->data;
	int i = 0;
	
	while(qset_ptr && i < scull_qset_item_num)
	{
		i += 1;
		prev = qset_ptr;
		qset_ptr = qset_ptr->next;
	}
	
	if(!qset_ptr)
	{
		prev->next = kmalloc( sizeof(struct scull_qset), GFP_KERNEL);
		qset_ptr = prev->next;
	}
	
	return qset_ptr;
}

static ssize_t scull_read(struct file * file_p, char __user * buff, size_t count, loff_t * pos)
{
	//printk("SCULL : SCULL_READ(), count=%d, *pos=%d\n", count, (*pos));
	struct scull_dev * scull_device = file_p->private_data;
	
	int quantum = scull_device->quantum, qset_buff = scull_device->qset;
	int qset_size = qset_buff * quantum;
	
	int scull_qset_item_num = (*pos) / qset_size;
	int remaining = (*pos) % qset_size;
	
	int buff_pos = remaining / quantum;
	int quant_pos = remaining % quantum;

	//printk("SCULL : read(), scull_qset_item_num=%d, remaining=%d, buff_pos=%d, quant_pos=%d\n", scull_qset_item_num, remaining, buff_pos, quant_pos);
	
	if( *pos > scull_device->size )
	{
		printk("FAILED SCULL : *pos is greater than device size\n");
		return -1;
	}
	
	if( *pos + count > scull_device->size )
	{
		count = scull_device->size - *pos;
		if(count == 0)
		{
			return count;
		}
	}
	
	struct scull_qset * scull_qset_item = get_scull_item(scull_device, scull_qset_item_num);
	
	if(!scull_qset_item || !scull_qset_item->data || !scull_qset_item->data[buff_pos])
	{
		//printk("scull_qset_item_num=%d\n", scull_qset_item_num);
		printk("FAILED SCULL : scull_qset_item is null\n");
		return -1;
	}
	
	if(count > quantum - quant_pos)
	{
		count = quantum - quant_pos;
	}
	
	if(copy_to_user(buff, scull_qset_item->data[buff_pos] + quant_pos, count))
	{
		printk("FAILED SCULL : copy_to_user()\n");
		return -1;
	}
	
	*pos += count;
	printk("SCULL : read %ld bytes\n", count);
	return count;
}

static ssize_t scull_write(struct file * file_p, const char __user * buff, size_t count, loff_t * pos)
{
	printk("SCULL : SCULL_WRITE(), count=%d, *pos=%d\n", count, (*pos));
	struct scull_dev * scull_device = file_p->private_data;
	
	int quantum = scull_device->quantum, qset_buff = scull_device->qset;
	int qset_size = qset_buff * quantum;
	
	int scull_qset_item_num = (*pos) / qset_size;
	int remaining = (*pos) % qset_size;
	
	int buff_pos = remaining / quantum;
	int quant_pos = remaining % quantum;
	
	//printk("SCULL : Getting scull item\n");
	
	struct scull_qset * scull_qset_item = get_scull_item(scull_device, scull_qset_item_num);
	
	if(!scull_qset_item)
	{
		//printk("scull_qset_item_num=%d\n", scull_qset_item_num);
		printk("FAILED SCULL : scull_qset_item is null\n");
		return -1;
	}
	
	//printk("SCULL : getting scull_qset_item->data\n");
	
	if(!scull_qset_item->data)
	{
		scull_qset_item->data = kmalloc( qset_buff * sizeof(void *), GFP_KERNEL);
	}
	
	if(!scull_qset_item->data[buff_pos])
	{
		//scull_qset_item->data[buff_pos] = kmem_cache_alloc(scull_quantum_cache, GFP_KERNEL);
		scull_qset_item->data[buff_pos] = kmalloc(quantum, GFP_KERNEL);
	}
	
	//printk("SCULL : Updating count\n");
	
	if(quantum - quant_pos < count)
	{
		count = quantum - quant_pos;
	}
	
	if(copy_from_user(scull_qset_item->data[buff_pos] + quant_pos, buff, count))
	{
		printk("FAILED SCULL : copy_from_user()");
		return -1;
	}
	
	*pos += count;
	scull_device->size += count;
	
	//printk("SCULL : dev->size=%d\n", scull_device->size);
	
	return count;
}

static void scull_trim(struct scull_dev * dev)
{
	printk("SCULL : scull_trim()");	
	struct scull_qset * qset_ptr = dev->data;
	struct scull_qset * next;
	int i;
	
	if(qset_ptr)
	{
		printk("SCULL : qset_ptr not null\nqset_ptr->qset=%d\n", dev->qset);
	}
	
	while(qset_ptr && dev->size)
	{
		for(i=0; i < dev->qset ; i++)
		{
			//kmem_cache_free(scull_quantum_cache, qset_ptr->data[i]);
			kfree(qset_ptr->data[i]);
		}
		printk("SCULL : After quantum free");
		kfree(qset_ptr->data);
		next = qset_ptr->next;
		kfree(qset_ptr);
		qset_ptr = next;
	}
	
	dev->data = kmalloc( sizeof(struct scull_qset), GFP_KERNEL);
	dev->quantum = SCULL_QUANTUM;
	dev->qset = SCULL_QSET;
	dev->size = 0;
	//printk("SCULL : dev->size=%d\nSCULL : scull_trim()\n", dev->size);
	
}

static int scull_open(struct inode * inode, struct file * filp)
{
	printk("SCULL : scull_open()\n");
	
	struct scull_dev * scull_device;
	scull_device = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = scull_device;
	if( (filp->f_flags & O_ACCMODE) == O_WRONLY )
	{
		scull_trim(scull_device);
	}
	
	return 0;
}

static int scull_release(struct inode * inode, struct file * filp)
{
	//printk("SCULL : scull_release()\n");
	return 0;
}

static int __init scull_init(void)
{
	printk("---------------------\n");
	if(allocate_device_num_dynmaically())
	{
		return -1;
	}
	
	scull_cdev = cdev_alloc();
	if (!scull_cdev)
	{
		printk("ERROR_SCULL : Failed to allocate cdev structure\n");
		unregister_chrdev_region(dev_num, 1);
		return -ENOMEM;
	}
	cdev_init(scull_cdev, &scull_fops);
	scull_cdev->owner = THIS_MODULE;
	
	if (cdev_add(scull_cdev, dev_num, 1) < 0) 
	{
		kfree(scull_cdev);
        	unregister_chrdev_region(dev_num, 1);
        	printk("SCULL FAILED : Failed to add cdev\n");
	        return -1;
        }
        //scull_trim(scull_cdev);
        printk("SCULL : Added char dev to kernel\n");
        
        //scull_quantum_cache = kmem_cache_create("scull_quantum_cache", SCULL_QUANTUM, 0, SLAB_HWCACHE_ALIGN, NULL);
        
        my_wq = create_singlethread_workqueue("scull_workqueue");
        INIT_WORK(work, interupt_handler, NULL);
        
        return 0;
}

static void __exit scull_exit(void)
{
	cdev_del(scull_cdev);
	printk("SCULL : Char dev deleted from kernel\n");
	unregister_chrdev_region(dev_num, 1);
	//printk("SCULL : Deallocated device with device num:%d, major number:%d\n", dev_num, MAJOR(dev_num));
}

module_init(scull_init);
module_exit(scull_exit);

