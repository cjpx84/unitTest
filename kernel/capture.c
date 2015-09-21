/*
* This file contains the caputre exception signal code
*
* (c) Copyright Tektronix Inc., All Rights Reserved
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/mnt.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>


#include <asm/signal.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>



#define CAPTURE_LEN 10

struct timeval  capture_tv;
struct capture_data *capture_hook;


/**
*  when the 'dst_task' receive a kernel cordump signal,check_signal will
*  dump the kernel backtrace and user space registers info
*/
static void check_signal(struct task_struct *dst_task,struct capture_data *data,int sig)
{
    struct pt_regs *regs;
    
    if(!dst_task)
        return;             	 	
    if(sig_kernel_coredump(sig)){
		do_gettimeofday(&capture_tv);
		printk("Time: sec %ld, usec %ld\n", (long)capture_tv.tv_sec, (long)capture_tv.tv_usec);
        /*check the signal sender and signal receiver*/
        printk("signal sender %s pid=%d,receiver %s pid=%d\n",
        current->comm,current->pid,dst_task->comm,dst_task->pid);
        
        regs = task_pt_regs(dst_task);
        show_regs(regs);
	#ifdef CONFIG_MNT_USER_BACK_TRACE
        if(dst_task->mm !=NULL){
            dump_user_stack(regs);
            backtrace_usrstack(dst_task);
        }
	#endif
    } 	
}

static struct capture_data capture_data = {
    .enabled = 1,
    .check_signal = check_signal,
};



/**
*  accord to proc file show the capture data enabled status  
*/
static int  capture_enable_read(char *page, char **start, off_t off, int count, int *eof,void *data)
{
    int len=0;
    struct capture_data *pdata = (struct capture_data*)data;
	len = sprintf(page,"enable =%d\n",pdata->enabled);
	return len;
}

/**
* accord to wirite proc file reset capture data enabled status
*/
static int capture_enable_write(struct file *file, const char __user *buffer, unsigned long  count, void *data)
{
    char buf[CAPTURE_LEN];
	int len;
	int enable =1;
	struct capture_data *pdata =(struct capture_data *)data;

	if(count > CAPTURE_LEN)
		len = CAPTURE_LEN;
	else
		len = count;

	if(copy_from_user(buf,buffer,len))
		return -EFAULT;
	if(sscanf(buf,"%d",&enable) == 1)
		pdata->enabled = !!enable;
	return len;
	
}

static int __init mnt_capture_init(void)
{
    struct proc_dir_entry *capture_root = NULL;
    struct proc_dir_entry *pde_enable = NULL;
	int ret=0;
	capture_hook = &capture_data;
	
    /**
    *  '/proc/capture/enable' file is used to control capture exception
    *  signal enabled or disabled
    */
    capture_root = proc_mkdir("capture",NULL);
    if( !capture_root){
        ret = -ENOMEM;
		goto root_fail;
    }
    pde_enable = create_proc_entry("enable",0600,capture_root);
	if( !pde_enable){
        ret = -EFAULT;
		goto enable_fail;
	}
	pde_enable->read_proc = capture_enable_read;
	pde_enable->write_proc = capture_enable_write;
	pde_enable->data = &capture_data;
	return 0;
	
enable_fail:
	printk("create enable entry fail\n");
	remove_proc_entry("capture",NULL);
root_fail:
    return ret;
}

static void __exit mnt_capture_exit(void)
{
    return ;
}


module_init(mnt_capture_init);
module_exit(mnt_capture_exit);

MODULE_AUTHOR(jiazhengli);
MODULE_LICENSE("GPL");



