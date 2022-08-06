#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alisa V <werewolf.luner@gmail.com>" );


static struct kobject *kobj;
char fname[256] = {0};

static ssize_t filename_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", fname);
}

static ssize_t filename_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	if (count > sizeof(fname)) {
		return -ENOMEM;
	}

	sscanf(buf, "%s\n", fname);
	return count;
	
//	return snprintf(fname, sizeof(fname), "%s", buf);
}

#if 1
static struct kobj_attribute filename_attribute = __ATTR(filename, S_IWUSR | S_IRUGO, filename_show, filename_store);
#else
static struct device_attribute filename = {
	.attr = {
		.name = "filename",
		.mode = S_IWUSR | S_IRUGO,
	},
	.show = filename_show,
	.store = filename_store,
};
#endif


static int __init infotecs_init( void )
{
	int error = 0;

	kobj = kobject_create_and_add("infotecs", kernel_kobj);
	if(!kobj)
		return -ENOMEM;

	error = sysfs_create_file(kobj, &filename_attribute.attr);
	if (error) {
		printk("Error while creating file %s (%d)\n", filename_attribute.attr.name, error);
	}

	return error;
}


static void __exit infotecs_exit( void )
{
	printk( "Goodbye, world!" );
	kobject_put(kobj);
}

module_init( infotecs_init );
module_exit( infotecs_exit ); 
