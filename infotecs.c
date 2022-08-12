#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/delay.h>
#include <linux/kthread.h>


DEFINE_SPINLOCK(sw_spinlock);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alisa V <werewolf.luner@gmail.com>" );

char *hello_string = "Hello from kernel module\n";
char *path = NULL;
int timer = 5;
static struct task_struct *thr;

int notify_param(const char *val, const struct kernel_param *kp)
{
	int res = 0;

	if (!val) {
		return -EINVAL;
	}
	printk(KERN_INFO "New path for writing is %s", val);	
	if (strlen(val) > PATH_MAX) {
		printk(KERN_INFO "Path length is too long (only %d is supported)\n", PATH_MAX);
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

int notify_param_timer(const char *val, const struct kernel_param *kp)
{
	int res = 0;
	int sec = 0;
	if (!val) {
		return -EINVAL;
	}
	sec = atoi(val);
	printk(KERN_INFO "New timer for writing is %d", sec);	
//	if (strlen(val) > PATH_MAX) {
//		printk(KERN_INFO "Path length is too long (only %d is supported)\n", PATH_MAX);
//		return -EINVAL;
//	}
		
        res = param_set_int(sec, kp); // Use helper for write variable
        if(res==0) {
                printk(KERN_INFO "Call back function called...\n");
                printk(KERN_INFO "New value of timer = %d\n", timer);
                return 0;
        }
        return -1;
}
const struct kernel_param_ops path_ops =
{
        .set = notify_param, // Use our setter ...
        .get = param_get_charp, // .. and standard getter
};

const struct kernel_param_ops timer_ops =
{
        .set = notify_param_timer, // Use our setter ...
        .get = param_get_int, // .. and standard getter
};
module_param_cb(path, &path_ops, &path, S_IRUGO|S_IWUSR);
module_param_cb(timer, &timer_ops, &timer, S_IRUGO|S_IWUSR);

static int writing_thread_func(void *arg) {

	struct file *f;
	loff_t pos = 0;
	
	allow_signal(SIGKILL); //this macro will allow to stop thread from userspace or kernelspace

	printk(KERN_INFO "I am thread: %s[PID = %d]\n", current->comm, current->pid);

	while(!kthread_should_stop()) {
		spin_lock(&sw_spinlock);
		printk("Writing thread executing on system CPU:%d \n",get_cpu());

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

		spin_unlock(&sw_spinlock);
		ssleep(timer);
		if (signal_pending(thr)) //if signal_pending function captures SIGKILL signal, then thread will exit
		    break;
	}
//thr = NULL;
//do_exit(0); //we use kthread_should_stop check, so no need in this

return 0;
}

static int __init infotecs_init( void )
{
	int error = 0;
	printk(KERN_INFO "Infotecs module is loaded\n");
	printk(KERN_INFO "path = %s, timer = %d\n", path, timer);

	if (!path)
		return 0;

//	thr = kthread_run(writing_thread_func, NULL, "string_writer");
	thr = kthread_create(writing_thread_func, NULL, "string_writer"); //using create&wake_up will save thr and allow kthread_stop if sigkill occured
	if (IS_ERR(thr)) {
		printk(KERN_INFO "ERROR: Cannot create thread\n");
		error = PTR_ERR(thr);
//	thr = NULL;
		return error;
	}
	get_task_struct(thr);
	wake_up_process(thr);

	printk("Thread running\n");

	return 0;
}


static void __exit infotecs_exit( void )
{
	if(thr) {
		kthread_stop(thr); //sets kthread_should_stop to true. Fails if sigkill occured (thr is not valid because already exited)
		printk(KERN_INFO "String_writer thread stopped\n");
		put_task_struct(thr);
	}
	printk(KERN_INFO "Goodbye, world!\n" );
}

module_init(infotecs_init);
module_exit(infotecs_exit); 
