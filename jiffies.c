#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/delay.h>
#include<linux/jiffies.h>
#include<linux/timer.h>
#include<linux/kthread.h>

#define TIMEOUT 6000

static struct task_struct *my_thread;

static struct timer_list my_timer;


void timer_callback(struct timer_list *timer){
	printk(KERN_INFO"jiffie value is : %lu\n", jiffies);
	mod_timer(&my_timer,msecs_to_jiffies(TIMEOUT));
}

int do_func(void *p){
	while(!kthread_should_stop()){
		printk(KERN_INFO"Inside kernel thread");
		msleep(2000);	
	}
	return 0;
}

MODULE_LICENSE("GPL");

static int test_init(void){
	printk(KERN_INFO"jiffies : %lu\n", jiffies);
	
	my_thread = kthread_run(do_func, NULL, "my thead");

	timer_setup(&my_timer, timer_callback, 0);
	mod_timer(&my_timer, jiffies+ msecs_to_jiffies(TIMEOUT));
	return 0;
}


static void test_exit(void){
	kthread_stop(my_thread);
	del_timer(&my_timer);
	printk(KERN_INFO"jiffies exiting\n");
}


module_init(test_init);
module_exit(test_exit);
