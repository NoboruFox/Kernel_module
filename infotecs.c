#include <linux/init.h>
#include <linux/module.h>
//#include <linux/device.h>
#include <linux/fs.h>
//#include <linux/file.h>
#include <linux/delay.h>
#include <linux/kthread.h>


DEFINE_SPINLOCK(sw_spinlock);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alisa V <werewolf.luner@gmail.com>" );

char *hello_string = "Hello from kernel module\n";
char *path = NULL;
char *timer = NULL;
int sec = 0;

static struct task_struct *thr; //thread

int notify_param_path(const char *val, const struct kernel_param *kp)
{
	int res = 0;

	if (!val) {
		return -EINVAL;
	}
	if (strlen(val) > PATH_MAX) {				//4096
		printk(KERN_INFO "Path length is too long (only %d is supported)\n", PATH_MAX);
		return -EINVAL;
	}
		
	printk(KERN_INFO "Path for writing is %s", val);	
        
	res = param_set_charp(val, kp); // Use helper for write variable. It takes val and sets it to 'arg' in kernel_param *kp struct
        if(res==0) {
                printk(KERN_INFO "Callback function called...\n");
                printk(KERN_INFO "New value of path = %s\n", path);
                return 0;
        }
        return -1;
}

int notify_param_timer(const char *val, const struct kernel_param *kp)
{
	int res = 0;
	int ret = 0;

	if (!val) {
		return -EINVAL;
	}
	ret = kstrtoint(val, 10, &sec); //convert string to int
	if (ret != 0)
		return ret;

	printk(KERN_INFO "Timer for writing is %d", sec);	
		
        res = param_set_int(val, kp); //  Change to "return param_set...." and remove next lines
        if(res==0) {
                printk(KERN_INFO "Call back function called...\n");
                printk(KERN_INFO "New value of timer = %d\n", sec); //why timer becomes (efault) here? (%s\n timer)
                return 0;
        }
        return -1; //return res!
}
const struct kernel_param_ops path_ops =
{
        .set = notify_param_path, // Use our setter ...
        .get = param_get_charp, // .. and standard getter
};

const struct kernel_param_ops timer_ops =
{
        .set = notify_param_timer, // Use our setter ...
        .get = param_get_int, // .. and standard getter
};

module_param_cb(path, &path_ops, &path, S_IRUGO|S_IWUSR); //accept parameter and write it to path, set callback functions for changing parameters from sys
module_param_cb(timer, &timer_ops, &timer, S_IRUGO|S_IWUSR);

static int writing_thread_func(void *arg) {

	struct file *f;
	loff_t pos = 0; //offset for writing in the file
	allow_signal(SIGKILL); //this macro will allow to stop thread from userspace or kernelspace

	printk(KERN_INFO "I am thread: %s[PID = %d]\n", current->comm, current->pid);

	while(!kthread_should_stop()) {
		
		printk("Writing thread executing on system CPU:%d \n",get_cpu());
		
		spin_lock(&sw_spinlock); //protect path and timer from changing while writing is in progress

		f = filp_open(path, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
		
		if (IS_ERR(f)){  //checks for error presense only. PTR_ERR returns the error type
			printk(KERN_INFO "create file error\n");
			return PTR_ERR(f);
		}
		
		else
			printk(KERN_INFO "Filp done\n");

		kernel_write(f, hello_string, strlen(hello_string), &pos);
		
		filp_close(f, NULL);

		ssleep(sec);
		spin_unlock(&sw_spinlock);

		if (signal_pending(thr)) //if signal_pending function captures SIGKILL signal, then thread will exit
		    break;
	}
//thr = NULL;
//do_exit(0); //we use kthread_should_stop check, so no need in this

return 0;
}

static int __init infotecs_init( void )
{
//	int error = 0;
	printk(KERN_INFO "Infotecs module is loaded\n");
	printk(KERN_INFO "path = %s, timer = %d\n", path, sec);

	if (!path)
		return 0;
//	thr = kthread_run(writing_thread_func, NULL, "string_writer");
	thr = kthread_create(writing_thread_func, NULL, "string_writer"); //using create&wake_up will save thr and allow kthread_stop if sigkill occured
	if (IS_ERR(thr)) {
		printk(KERN_INFO "ERROR: Cannot create thread\n");
//		error = PTR_ERR(thr);
		thr = NULL;
		return PTR_ERR(thr);
	}
	get_task_struct(thr); //thr struct is saved here to make kthread_stop able to stop thread if sigkill occured
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
