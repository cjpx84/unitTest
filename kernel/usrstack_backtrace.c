 /*
 * This file implemented user stack backtrace 
 *
 * (c) Copyright Tektronix Inc., All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


 #include <linux/delay.h>
 #include <linux/hardirq.h>
 #include <linux/init.h>
#include <linux/stacktrace.h>
 #include <linux/uaccess.h>
 #include <linux/sched.h>

#define MAX_FUNC_TRACK          32 /* max backtrace level  */
 typedef struct
 {
	 unsigned long RetAddr;  /*  Return address*/
	 unsigned long sfp;      /* The stack frame pointer*/
 }CALL_STACK_FUNC_INFO;
 
 typedef struct
 {
	 unsigned long tid;                          /* thread id */
	 CALL_STACK_FUNC_INFO Funcs[MAX_FUNC_TRACK]; /*back trace result*/
	 unsigned long NestCount;					  /* The actual fucntion called level */
 }CALL_STACK_INFO;
 

#ifdef CONFIG_ARM

/**
 * The registers we're interested in are at the end of the variable
 * length saved register structure. The fp points at the end of this
 * structure so the address of this struct is:
 * (struct frame_tail_mapcs *)(xxx->fp)-1
 */
	
struct frame_tail {
	struct frame_tail *fp;
	unsigned long sp;
	unsigned long lr;
} __attribute__((packed));

struct frame_tail_nomapcs {
	struct frame_tail *fp;
	unsigned long lr;
} __attribute__((packed));



static void show_user_trace(struct task_struct *pp,struct pt_regs* regs,CALL_STACK_INFO *stackinfo)
{
	struct frame_tail *tail;
	unsigned long framebuf[3] = {0};
	unsigned long addrmask;
	int nomapcs = 0;
	int nestcount = 0;
    int framelen;
	
	tail = ((struct frame_tail *) regs->ARM_fp) - 1;
	
	/*record first level function call*/
	stackinfo->tid = pp->pid;
	stackinfo->Funcs[nestcount].sfp = regs->ARM_fp;
	stackinfo->Funcs[nestcount].RetAddr= regs->ARM_pc;
	nestcount++;

	if (!access_ok(VERIFY_READ, tail, sizeof(framebuf))){
		return;
	}
	if (__copy_from_user_inatomic(framebuf, tail, sizeof(framebuf))){
		return;
	}


	/**
	* the address's high byte is the same, this address must be fp/sp, so
	* asserts that the stack frame is the standard stack frame .Standard 
	* stack frame header contains fp/sp/lr, otherwise the stack frame is the 
	* optimize the stack frame, contains only fp/lr 
	*/
	addrmask =  0xff << ((sizeof(unsigned long) - sizeof(char))<<3);
	if((framebuf[0] &  addrmask) == (regs->ARM_fp & addrmask)){
		framelen = sizeof(struct frame_tail);
	}
	else{
		framelen = sizeof(struct frame_tail_nomapcs);
		nomapcs = 1;
		/**/
		tail = (struct frame_tail *)((unsigned long)tail + framelen);
	}
	while (tail && !((unsigned long) tail & 3) && nestcount < MAX_FUNC_TRACK){
		memset(framebuf,0,sizeof(framebuf));
		if (!access_ok(VERIFY_READ, tail, sizeof(framebuf))){
			break;
		}
		if (__copy_from_user_inatomic(framebuf, tail, framelen)){
			break;
		}
		if(!nomapcs){
			stackinfo->Funcs[nestcount].sfp = framebuf[0];
			stackinfo->Funcs[nestcount].RetAddr= framebuf[2];
		}
		else{
			stackinfo->Funcs[nestcount].sfp = framebuf[0];
			stackinfo->Funcs[nestcount].RetAddr= framebuf[1];
		}
		nestcount++;
		if(!nomapcs)
			tail = (struct frame_tail *)(framebuf[0] - framelen);
		else
			tail = (struct frame_tail *)(framebuf[0] - sizeof(unsigned long));
	}
	stackinfo->NestCount = nestcount;
}

static void usr_stack_trace(CALL_STACK_INFO * stackinfo,struct task_struct *p)
{
	struct pt_regs* regs;

	preempt_disable();

	if(p == NULL){
	    p = current;
	}
	
	regs = task_pt_regs(p);
	if(!regs || !user_mode(regs))
	{
		printk("Process %d is not a user process!\n",p->pid);
		preempt_enable();
		return;
	}
	show_user_trace(p,regs,stackinfo);
	preempt_enable();
}


void backtrace_usrstack(struct task_struct *p)
{
	CALL_STACK_INFO usrstack={0};
	int idx=0;

	usr_stack_trace(&usrstack,p);
	if(usrstack.NestCount == 0)
		return;
		
	printk("\nPid: %lu\n", usrstack.tid);
	printk("Usrstack Call Trace:\n");
	for(idx=0; idx<usrstack.NestCount; idx++){
		printk("[<%lx>] 0x%lx\n", usrstack.Funcs[idx].sfp,usrstack.Funcs[idx].RetAddr);
	}
}


/**
* dump user stack start from sp,end at sp+4096
*/
void dump_user_stack(struct pt_regs *regs)
{
    unsigned long sp = regs->ARM_sp;
    unsigned long buffer[4];
    unsigned long total_len =4*1024;
    unsigned long count= total_len;
	int buflen = sizeof(buffer);
	int i=0;

	/*just dump user stack*/
	 if(!user_mode(regs)){
        printk("just dump user stack\n");
		return;
	 }
	 
    while(count > 0){
        if (!access_ok(VERIFY_READ, sp, buflen))
            break;
        if (__copy_from_user_inatomic(buffer, (void *)sp, buflen))
            break;
		printk("%08lx: ",sp);
		for( i=0;i<4;i++)
		    printk("%08lx ",buffer[i]);
		printk("\n");
		count -= buflen;
		sp += buflen;
		
    }
    
}

#else/*CONFIG_ARM*/

void backtrace_usrstack(struct task_struct *p)
{
}

void dump_user_stack(struct pt_regs *regs)
{
}
#endif

EXPORT_SYMBOL(backtrace_usrstack);

EXPORT_SYMBOL(dump_user_stack);


