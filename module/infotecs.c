#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kthread.h>

DEFINE_SPINLOCK(sw_spinlock);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alisa V <werewolf.luner@gmail.com>" );

char *hello_string = "Hello from kernel module\n";
char *path = NULL;
char *timer = NULL;		
int sec = 0;

static struct task_struct *thr;

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
 
	res = param_set_charp(val, kp);
	if(res==0) {
		printk(KERN_INFO "Path for writing = %s\n", path);
		return 0;
        }
	return res;
}

int notify_param_timer(const char *val, const struct kernel_param *kp)
{
	int res = 0;
	int ret = 0;

	if (!val) {
		return -EINVAL;
	}
	ret = kstrtoint(val, 10, &sec); 
	if (ret != 0)
		return ret;

	res = param_set_int(val, kp); 
	if(res==0) {
		printk(KERN_INFO "Value of timer = %d\n", sec);
		return 0;
        }
        return res;
}
const struct kernel_param_ops path_ops =
{
	.set = notify_param_path,
	.get = param_get_charp,
};

const struct kernel_param_ops timer_ops =
{
	.set = notify_param_timer,
	.get = param_get_int,
};

module_param_cb(path, &path_ops, &path, S_IRUGO|S_IWUSR);
module_param_cb(timer, &timer_ops, &timer, S_IRUGO|S_IWUSR);

static int writing_thread_func(void *arg) {

	struct file *f;
	loff_t pos = 0;
	int i = 0;
	allow_signal(SIGKILL); //this macro will allow to stop thread from userspace or kernelspace

	printk(KERN_INFO "Thread %s [PID = %d] executing on system CPU: %d\n", current->comm, current->pid, get_cpu());

	while(!kthread_should_stop()) {
		
		if (!path || !timer) {
			printk(KERN_INFO "No path and/or timer value provided, waiting for them both while sleeping...\n");
			ssleep(5);
			continue;
		}

		spin_lock(&sw_spinlock); //protect path and timer from changing while writing is in progress

		f = filp_open(path, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
		
		if (IS_ERR(f)){
			printk(KERN_INFO "File creation error\n");
			return PTR_ERR(f);
		}
		
		kernel_write(f, hello_string, strlen(hello_string), &pos);
		
		filp_close(f, NULL);
		
		printk (KERN_INFO "Writing done\n");
		
		for (i = 0; i <= sec; i++) {	//if stop occured while thread sleeping, it should stop instead of waiting until it wakes
			if (kthread_should_stop())
				break;
			ssleep(1);
		}
		
		spin_unlock(&sw_spinlock);

		if (signal_pending(thr)) //if signal_pending function captures SIGKILL signal, then thread will exit
		    break;
	}

	return 0;
}

static int __init infotecs_init( void )
{
	printk(KERN_INFO "Infotecs module is loaded\n");
	printk(KERN_INFO "path = %s, timer = %s\n", path, timer);

	thr = kthread_create(writing_thread_func, NULL, "string_writer"); 
	if (IS_ERR(thr)) {
		printk(KERN_INFO "ERROR: Cannot create thread\n");
		thr = NULL;
		return PTR_ERR(thr);
	}
	get_task_struct(thr);
	wake_up_process(thr);

	return 0;
}


static void __exit infotecs_exit( void )
{
	if(thr) {
		kthread_stop(thr);
		put_task_struct(thr);
	}
	printk(KERN_INFO "Goodbye, world!\n" );
}

module_init(infotecs_init);
module_exit(infotecs_exit); 
