/*
* This file contains the kenel maintain info code
*
* (c) Copyright Tektronix Inc., All Rights Reserved
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/


#include<linux/kthread.h>
#include<linux/completion.h>
#include<linux/timer.h>

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/module.h>

#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/tick.h>

#include<linux/mnt.h>




static int show_mnt(struct seq_file *p, void *v)
{

    mnt_dump_task_log(p,v);
    return 0;
}

static int mnt_open(struct inode *inode, struct file *file)
{

    return single_open(file, show_mnt, NULL);
}

static const struct file_operations proc_mnt_operations = {
    .open		= mnt_open,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
};


static unsigned long long prev_time =0;
static unsigned long long cur_time =0;
static unsigned short first_flag =0;

/**
* calculate each thread cpu percent
*/
static void calculate_cpu_percent(void)
{
    unsigned long long delta_total_time =0;
    struct task_struct *g;
    struct task_struct *p;
    unsigned long long delta_time =0;
    cur_time = sched_clock();
	
    /**
    * first exec this function ,delta_total_time equal cur_time  
    */
	
    delta_total_time = cur_time - prev_time;
    prev_time = cur_time;
    
    rcu_read_lock();
    do_each_thread (g, p) {
            if(first_flag ==0){
                p->cpu_percent = 0;
            }else{
                delta_time = (p->se.sum_exec_runtime - p->prev_sum_exec_runtime)*100;
				p->cpu_percent =nsecs_to_jiffies(delta_time) / nsecs_to_jiffies(delta_total_time);
			    	
            }
	        
	 p->prev_sum_exec_runtime = p->se.sum_exec_runtime;
	
	 	
    } while_each_thread(g, p);
	rcu_read_unlock();
	

	if(first_flag == 0)
		first_flag =1;

}

DECLARE_COMPLETION(cpu_comp);
struct timer_list cpu_timer;

void mnt_cpu_complete(unsigned long arg)
{ 
    complete(&cpu_comp);
    cpu_timer.expires = jiffies +500;
    add_timer(&cpu_timer);
	
}


static int __init cpu_timer_init(void)
{
    init_timer(&cpu_timer);
    cpu_timer.function = mnt_cpu_complete;
    cpu_timer.data = 0;
    cpu_timer.expires = jiffies +500;
    add_timer(&cpu_timer);
    return 0;
}

static void __exit exit_cpu_timer(void)
{
    del_timer(&cpu_timer);
}
int kthread_cpu_percent(void *data)
{
    while(!kthread_should_stop())
    {
        wait_for_completion_interruptible(&cpu_comp);
        calculate_cpu_percent();
    }
    return 0;
}


static int show_thread_info(struct seq_file *f,void *v)
{

    struct task_struct *g;
    struct task_struct *p;
	
    seq_printf(f, "\nkernel thread info:\n\n");
    seq_printf(f, "\nthreadid    threadname              priority    runtime             cpu(%%)    voluntary-cs    involuntary-cs    last-start-sched-time\n");

	rcu_read_lock();
    do_each_thread (g, p) {
		if(p->mm == NULL)
            seq_printf(f,"%-8d    %-20s    %-8d    %-16llu    %-6d    %-16lu    %-16lu    %14Ld.%06ld\n",
            p->pid,p->comm,p->prio,p->se.sum_exec_runtime,p->cpu_percent,p->nvcsw,p->nivcsw,MNT_SPLIT_NS(p->se.exec_start));    
    } while_each_thread(g, p);



    seq_printf(f, "\nuser thread info:\n\n");
    seq_printf(f, "\nthreadid    threadname              priority    runtime             cpu(%%)    voluntary-cs    involuntary-cs    last-start-sched-time\n");
    do_each_thread (g, p) {
		 if(p->mm != NULL)
            seq_printf(f,"%-8d    %-20s    %-8d    %-16llu    %-6d    %-16lu    %-16lu    %14Ld.%06ld\n",
                        p->pid,p->comm,p->prio,p->se.sum_exec_runtime,p->cpu_percent,p->nvcsw,p->nivcsw,MNT_SPLIT_NS(p->se.exec_start));
    
    } while_each_thread(g, p);

	rcu_read_unlock();
    
    return 0;
}

static int thread_info_open(struct inode *inode,struct file *file)
{
    return single_open(file, show_thread_info, NULL);
}

static const struct file_operations proc_threadinfo_operations = {
    .open           = thread_info_open,
    .read           = seq_read,
    .llseek         = seq_lseek,
    .release        = single_release,
};

static int __init proc_mnt_init(void)
{
    struct proc_dir_entry* mnt_root;
    struct task_struct *kthread = NULL;
    mnt_root = proc_mkdir("mnt",NULL);
    
    if(mnt_root != NULL){
        proc_create("monitor_info", 0, mnt_root, &proc_mnt_operations);
        proc_create("thread_info", 0, mnt_root, &proc_threadinfo_operations);   
    }	

    kthread = kthread_create(kthread_cpu_percent,NULL,"cpu_cal");
    if(kthread != NULL)
        wake_up_process(kthread);
    
    cpu_timer_init();

	
    return 0;
}

static void __exit proc_mnt_exit(void)
{
    exit_cpu_timer();
    return;
}
module_init(proc_mnt_init);
module_exit(proc_mnt_exit);

MODULE_AUTHOR(jiazhengli);
MODULE_LICENSE("GPL");

