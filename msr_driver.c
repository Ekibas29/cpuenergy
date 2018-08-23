#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>

#define DEV_MAJOR	202

static struct class *dev_class;

MODULE_DESCRIPTION("Simple MSR read driver");
MODULE_LICENSE("GPL");
MODULE_VERSION( "1.0" );

static int dev_open(struct inode *inode, struct file *file)
{
	unsigned int cpu = iminor(file->f_path.dentry->d_inode);
	int ret = 0;

	cpu = iminor(file->f_path.dentry->d_inode);

	if (cpu >= nr_cpu_ids || !cpu_online(cpu)) {
		ret = -ENXIO;
	}
	
	return ret;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;
	ssize_t bytes = 0;

	if (count % 8)
		return -EINVAL;

	for (; count; count -= 8) {
		err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
		if (err)
			break;
		if (copy_to_user(tmp, &data, 8)) {
			err = -EFAULT;
			break;
		}
		tmp += 2;
		bytes += 8;
	}

	return bytes ? bytes : err;
}

static const struct file_operations dev_fops = {
   .owner = THIS_MODULE,
   .open = dev_open,
   .read  = dev_read,
};

static int __init dev_init(void)
{
	int err = 0, pkgid, prev_pkgid = -1, i = 0;
	struct device *dev;
	struct cpuinfo_x86 *cpu;
	
	if (register_chrdev(DEV_MAJOR, "msr_pkg", &dev_fops) < 0) {
		printk(KERN_ERR "msr_driver: can't get major %d\n",
		       DEV_MAJOR);
		err = -EBUSY;
		goto err;
	}
	
	dev_class = class_create(THIS_MODULE, "msr_pkg");
	if (IS_ERR(dev_class)) {
		err = PTR_ERR(dev_class);
		printk(KERN_ERR "msr_driver: can't create device class");
		goto err_chrdev;
	}
	
	for_each_online_cpu(i) {
		pkgid = topology_physical_package_id(i);
		
		if(pkgid != prev_pkgid) {
			cpu = &cpu_data(i);
			if (!cpu_has(cpu, X86_FEATURE_MSR)) {
				printk(KERN_ERR "msr_driver: MSR feature is not supported\n");
				err = -EIO;
				goto err_class;
			}
			
			prev_pkgid = pkgid;
			dev = device_create(dev_class, NULL, MKDEV(DEV_MAJOR, i), NULL,
								"msr_pkg%d", pkgid);
			if (IS_ERR(dev)) {
				err = PTR_ERR(dev);
				printk(KERN_ERR "msr_driver: can't create device");
				goto err_devcrt;
			}
			
			printk(KERN_INFO "msr_driver: successful device msr_pkg%d creation", pkgid);
		}
	}
	
	err = 0;
	goto err;

err_devcrt:
	i = 0;
	for_each_online_cpu(i) {
		pkgid = topology_physical_package_id(i);
		if(pkgid > prev_pkgid)
			break;
		device_destroy(dev_class, MKDEV(DEV_MAJOR, i));
	}
err_class:
	class_destroy(dev_class);
err_chrdev:
	unregister_chrdev(DEV_MAJOR, "msr_pkg");
err:
	return err;
}

static void __exit dev_exit(void)
{
	int i = 0, pkgid, prev_pkgid = -1;
	for_each_online_cpu(i) {
		pkgid = topology_physical_package_id(i);
		if(pkgid != prev_pkgid) {
			prev_pkgid = pkgid;
			device_destroy(dev_class, MKDEV(DEV_MAJOR, i));
		}
	}
	class_destroy(dev_class);
	unregister_chrdev(DEV_MAJOR, "msr_pkg");
}

module_init(dev_init);
module_exit(dev_exit);
