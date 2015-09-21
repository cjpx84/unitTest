/**
*
* (c) Copyright Tektronix Inc., All Rights Reserved
*
* Author: namo (nan.mo@tektronix.com)
*
*
* Author           Date               Tracking No.    Description of Changes
* -----------------------------------------------------------------------------
* Nan.mo          30-Sep-2014                        Create   
* jiazheng.li      15-Oct-2014                         Modify
*/

#ifndef _MNT_H_
#define _MNT_H_


#ifdef CONFIG_MNT
extern long long mnt_nsec_high(unsigned long long nsec);
extern unsigned long mnt_nsec_low(unsigned long long nsec);


#define MNT_SPLIT_NS(x) mnt_nsec_high(x), mnt_nsec_low(x)


#ifdef CONFIG_MNT_TASK_SWITCH
#define MNT_TASK_NAME_SIZE  32

/*
    The head of a log buffer,  a log buffer will start with this head and 
    followed by data array.
*/
struct mnt_data_hd 
{
    unsigned int base_addr;  /* this buffer's memory address, 
                                 the start of this struct date */
    unsigned int total_cnt;  /* the max log element count in this buffer*/
    unsigned int head_cnt;   /* the current log element count in this buffer, 
                                empty */
    unsigned int reserved;
    char data[1];            /*the log element array start from here */
};

/*
    maintain monitor handler
*/
struct mnt_monitor 
{
    unsigned int init_flag;
    struct mnt_data_hd* task_switch;
    struct mnt_data_hd* int_log;  
};

/*
    the data array describe int  event.
*/
struct int_log_data 
{
    unsigned int direct;                /* ISR in or out */
    unsigned int number;                /* interrupt number */
    unsigned long long  timestamp;      /* timestamp */
    unsigned int reserved;
    
};

/*
    the data array describe the task switch event.
*/
struct task_switch_data 
{
    unsigned int prev;       /* previous task ID*/
    unsigned int next;       /* next task ID*/
    char prevname[MNT_TASK_NAME_SIZE];
    char nextname[MNT_TASK_NAME_SIZE];
    unsigned long long timestamp;       /* timestamp */
    unsigned int reserved;    
};


extern struct mnt_monitor mnt_hdl;
extern void mnt_task_switch_hook(struct task_struct *prev, struct task_struct *next);
extern void mnt_dump_task_log(struct seq_file *p,void *v);

#endif/*CONFIG_MNT_TASK_SWITCH*/
/**
* exception signal capture interface
*/
#ifdef CONFIG_MNT_SIGNAL_CAPTURE

struct capture_data{
    int enabled;
    void (*check_signal)(struct task_struct *p,struct capture_data *data,int sig);
};	
    extern struct capture_data *capture_hook;
#endif/*CONFIG_MNT_SIGNAL_CAPTURE*/

#ifdef CONFIG_MNT_USER_BACK_TRACE
extern void dump_user_stack(struct pt_regs *regs);
void backtrace_usrstack(struct task_struct *p);

#endif

#endif/*CONFIG_MNT*/
#endif

