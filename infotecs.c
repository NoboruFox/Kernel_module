#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alisa V <werewolf.luner@gmail.com>" );

#define PATH_LENGTH 256

//struct kobject binds sysfs and the kernel. Represents a kernel object (device etc)
static struct kobject *kobj;
char fname[PATH_LENGTH] = {0};
char *path = NULL;
char *hello_string = "Hello from kernel module\n";

#if 1 
int notify_param(const char *val, const struct kernel_param *kp)
{
	int res = 0;

	if (!val) {
		return -EINVAL;
	}
	
	if (strlen(val) > PATH_LENGTH) {
		printk(KERN_INFO "Path length is too long\n");
		return -EINVAL;
	}
		
        res = param_set_charp(val, kp); // Use helper for write variable
        if(res==0) {
                printk(KERN_INFO "Call back function called...\n");
                printk(KERN_INFO "New value of path = %s\n", path);
                return 0;
        }
        return -1;
}

const struct kernel_param_ops my_param_ops =
{
        .set = notify_param, // Use our setter ...
        .get = param_get_charp, // .. and standard getter
};

module_param_cb(path, &my_param_ops, &path, S_IRUGO|S_IWUSR);

#else
module_param(path, charp, S_IRUGO|S_IWUSR);

#endif

//callback from struct kobj_attribute. When read
static ssize_t filename_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	ssize_t count = 0;
	count = sprintf(buf, "%s\n", fname); //from fname to buf
	if (path)
		count += sprintf(buf + count, "%s\n", path); //from fname to buf

	return count;
}

//when write
static ssize_t filename_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	if (count > sizeof(fname)) {
		return -ENOMEM;
	}

	sscanf(buf, "%s\n", fname); //reading from buf to fname
	return count;
	
//	return snprintf(fname, sizeof(fname), "%s", buf);
}

#if 1

//creating an attribute using __ATTR macro (for file creation)
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
struct kobj_attribute {
 struct attribute attr;		//the file to be created
 ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr, char *buf);		// the pointer to the function (callback) that will be called when the file is read in sysfs
 ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);	//the pointer to the function (callback) which will be called when the file is written in sysfs
};
#endif

static int __init infotecs_init( void )
{
	struct file *f;
	loff_t pos = 0;
	int error = 0;
	printk(KERN_INFO "Infotecs module is loaded\n");
	printk(KERN_INFO "path = %s\n", path);

//creating directory "infotecs" in sysfs (/sys/kernel/ - provided by "kernel_kobj" parameter) 
	kobj = kobject_create_and_add("infotecs", kernel_kobj);
	if(!kobj)
		return -ENOMEM; //If the kobject was not able to be created, NULL will be returned.??

//creating a file in that directory for r/w, size of a file is 4096 bytes (page_size)
	error = sysfs_create_file(kobj, &filename_attribute.attr);
	if (error) {
		printk("Error while creating file %s (%d)\n", filename_attribute.attr.name, error);
	}

//	return error;
	
	f = filp_open(path, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
	if (IS_ERR(f)){
        printk(KERN_INFO "create file error\n");
        return -1;
	}
	if (f){
		printk(KERN_INFO "Filp done\n");
	}

	kernel_write(f, hello_string, strlen(hello_string), &pos);
	
	filp_close(f, NULL);

	return 0;
}


static void __exit infotecs_exit( void )
{
	printk( "Goodbye, world!" );
	kobject_put(kobj);
}

module_init( infotecs_init );
module_exit( infotecs_exit ); 
