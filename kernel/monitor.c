/**
* Deadlock Detecting Interface Implement.
*
* (c) Copyright Tektronix Inc., All Rights Reserved
*
* Author: namo (nan.mo@tektronix.com)
*
*
*  Revision History:
*
*  Author           Date               Tracking No.    Description of Changes
*  -----------------------------------------------------------------------------
*  Nan.mo          22-Sep-2014                        Create draft  
* jiazheng.li      5-Nov-2014                          Modify
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/seq_file.h>
#include <linux/mnt.h>


char mnt_data_buf[5*1024];   //TODO: this buffer should change to a fixed memoey space

struct mnt_monitor mnt_hdl;

#define MNT_DATA_BUF_ADDRESS     (&mnt_data_buf[0])
#define MNT_DATA_BUF_SIZE        5*1024  // to be adjustment

#define TASK_SWITCH_BUF_ADDRESS  MNT_DATA_BUF_ADDRESS
#define TASK_SWITCH_BUF_SIZE     4*1024 // to be adjustment

#define INT_LOG_BUF_ADDRESS     (TASK_SWITCH_BUF_ADDRESS + TASK_SWITCH_BUF_SIZE)
#define INT_LOG_BUF_SIZE        1024 // to be adjustment

#define MNT_RESET_FLAG_ADDR     (INT_LOG_BUF_ADDRESS + INT_LOG_BUF_SIZE)
#define MNT_RESET_FLAG_SIZE     4

#define MNT_DATA_BUF_END        (MNT_RESET_FLAG_ADDR + MNT_RESET_FLAG_SIZE)
#define MNT_INIT_MAGIC          0x12348765

#define MNT_POWER_ON_WDT        0x5a5a1234
#define MNT_POWER_ON_ASSERT     0x5a5a4321
#define MNT_POWER_ON_NORMAAL    0x5a5a5678

#define HOOK_INT_FLAG_IN        0x55555555
#define HOOK_INT_FLAG_OUT       0xAAAAAAAA
#define HOOK_INT_DIR_IN         1
#define HOOK_INT_DIR_OUT        0
#define HOOK_RESERVE_FLAG       0x5A5A5A5A


/*******************************************************************************
 * FUNCTION
 *   mnt_task_switch_hook
 *
 * DESCRIPTION
 *     A hook function called by kernel schdule routine, will record thread 
 * switch info, with the timestamp.
 *   
 *  CALLER
 *     __schedule() in sched.c
 *
 * PARAMETERS
 * struct task_struct * prev -- the current thread id.
 * struct task_struct * next -- the thread that is going to switch id.
 *
 * RETURNS
 *  None.
 */
void mnt_task_switch_hook(struct task_struct *prev, struct task_struct *next)
{
    struct mnt_data_hd  *ptask_hd;
    struct task_switch_data   *pdata;

    if (MNT_INIT_MAGIC != mnt_hdl.init_flag)
    {
        return ;
    }
    
    ptask_hd = (struct mnt_data_hd*)mnt_hdl.task_switch;
    pdata = (struct task_switch_data *)ptask_hd->data +
                ptask_hd->head_cnt;

    pdata->prev = prev->pid;
    pdata->next = next->pid;
    memcpy(pdata->prevname,prev->comm,MNT_TASK_NAME_SIZE);
    memcpy(pdata->nextname,next->comm,MNT_TASK_NAME_SIZE);
    pdata->timestamp = sched_clock();
    pdata->reserved = HOOK_RESERVE_FLAG;
    
    if (ptask_hd->head_cnt == ptask_hd->total_cnt)
    {
        ptask_hd->head_cnt = 0;
    }
    else
    {
       ptask_hd->head_cnt++;
    }
}

/*******************************************************************************
 * FUNCTION
 *   mnt_int_switch_hook
 *
 * DESCRIPTION
 *     A hook function called by kernel ISR routine, will record interrupt 
 * calling info.
 *
 * PARAMETERS
 *  unsigned int num -- the intrrupt number.
 *  unsigned int dir -- the ISR in or out.
 *
 * RETURNS
 *  None.
 */
void mnt_int_switch_hook(unsigned int num, unsigned int dir)
{
    struct mnt_data_hd  *pint_log;
    struct int_log_data   *pdata;

    if (MNT_INIT_MAGIC != mnt_hdl.init_flag)
    {
        return ;
    }
    
    pint_log = (struct mnt_data_hd*)mnt_hdl.int_log;
    pdata = (struct int_log_data *)pint_log->data +
                pint_log->head_cnt;

    if (HOOK_INT_DIR_IN == dir)
    {
        pdata->direct= HOOK_INT_FLAG_IN;
    }
    else
    {
        pdata->direct= HOOK_INT_FLAG_OUT;
    }

    pdata->number= num;
    pdata->timestamp= jiffies;
    pdata->reserved = HOOK_RESERVE_FLAG;

    if (pint_log->head_cnt == pint_log->total_cnt)
    {
        pint_log->head_cnt = 0;
    }
    else
    {
        pint_log->head_cnt++;
    }
}


/*******************************************************************************
 * FUNCTION
 *   mnt_set_reset_flag
 *
 * DESCRIPTION
 *     set reset flag, to indicate the reset reason.
 *
 * CALLER  
 *  wtd timer, assert routine, reset routine
 *
 * PARAMETERS
 *  unsigned int num -- the intrrupt number.
 *  unsigned int dir -- the ISR in or out.
 *
 * RETURNS
 *  None.
 */
void mnt_set_reset_flag(unsigned int value)
{
    /* MNT_POWER_ON_WDT MNT_POWER_ON_ASSERT */
    *(unsigned int*)MNT_RESET_FLAG_ADDR = value;
}

/*******************************************************************************
 * FUNCTION
 *   mnt_save_data_to_file
 *
 * DESCRIPTION
 *    save mnt data form memory to a file : 
 *    MNT_DATA_BUF_ADDRESS and MNT_DATA_BUF_SIZE
 *
 * PARAMETERS
 *  None.
 *
 * RETURNS
 *  None.
 */
void mnt_save_data_to_file(void)
{
    
    printk("mnt_hdl_seve_log_to_file \n");
    
}


/**
*  dump task switch log for proc show
*/
void mnt_dump_task_log(struct seq_file *p,void *v)
{
    struct mnt_data_hd  *phd;
    struct task_switch_data   *pswitch;  
    signed int curcnt;
    
    if (MNT_INIT_MAGIC != mnt_hdl.init_flag){
        seq_printf(p,"mnt_hdl_dump_task_log: error, mnt  not inited!!\n");
        return ;
    }
    
    /* start dump task switch*/
    phd = (struct mnt_data_hd *)mnt_hdl.task_switch;
    curcnt = phd->head_cnt;
    seq_printf(p,"mnt dump task switch:total %d start(%d)....\n",phd->total_cnt, curcnt);
    seq_printf(p,"\nindex   prev-task-name         prev-task-id   next-task-name         next-task-id        timestamp           reserved\n");
    
    do {
        if (curcnt == 0){
            curcnt = phd->total_cnt;
        } else{
            curcnt--;
        }
        
        pswitch = (struct task_switch_data *)phd->data + curcnt;
        if (HOOK_RESERVE_FLAG == pswitch->reserved){
            seq_printf(p,"%-4d    %-20s    %-12d    %-20s    %-12d    %14Ld.%06ld    0x%08x \n", 
            curcnt,pswitch->prevname,pswitch->prev, pswitch->nextname,pswitch->next, MNT_SPLIT_NS(pswitch->timestamp), pswitch->reserved);
        }        
    } while (curcnt != phd->head_cnt);

}


/*******************************************************************************
 * FUNCTION
 *   mnt_hdl_monitor_init
 *
 * DESCRIPTION
 *    intialization the maintain module handler 
 *
 * PARAMETERS
 *  None.
 *
 * RETURNS
 *  None.
 */
static int __init mnt_monitor_init(void)
{
    struct mnt_monitor *pmnt_hdl = &mnt_hdl;
    struct mnt_data_hd  *pdata_hd;
    unsigned int power_flag = *(unsigned int*)MNT_RESET_FLAG_ADDR;

    printk("monan: mnt_monitor_init enter.\n");

    /* save data to file if necessary  */
    if ((power_flag == MNT_POWER_ON_WDT) || (power_flag == MNT_POWER_ON_ASSERT))
    {
        mnt_save_data_to_file();
    }

    /* init task switch head */
    memset(TASK_SWITCH_BUF_ADDRESS, 0x00, TASK_SWITCH_BUF_SIZE );
    pdata_hd = (struct mnt_data_hd  *)TASK_SWITCH_BUF_ADDRESS;
    pdata_hd->base_addr = (unsigned int)pdata_hd;
    pdata_hd->total_cnt = (TASK_SWITCH_BUF_SIZE - sizeof(struct mnt_data_hd))/
                            sizeof(struct task_switch_data) - 1;

    pdata_hd->head_cnt = 0;
    pdata_hd->reserved = 0;
    pmnt_hdl->task_switch = pdata_hd;

    printk("task switch: 0x%x, 0x%x, 0x%x\n", (int)pdata_hd, 
                (int)pdata_hd->base_addr, (int)pdata_hd->total_cnt);
    /* init int log head */
    memset(INT_LOG_BUF_ADDRESS, 0x00, INT_LOG_BUF_SIZE );
    pdata_hd  = (struct mnt_data_hd  *)INT_LOG_BUF_ADDRESS;
    pdata_hd->base_addr = (unsigned int)pdata_hd;
    pdata_hd->total_cnt = (INT_LOG_BUF_SIZE - sizeof(struct mnt_data_hd))/
                            sizeof(struct int_log_data) - 1;
    pdata_hd->head_cnt = 0;
    pdata_hd->reserved = 0;
    pmnt_hdl->int_log  = pdata_hd;

    printk("int log: 0x%x, 0x%x, 0x%x\n", (int)pdata_hd,
                             (int)pdata_hd->base_addr, pdata_hd->total_cnt);
    pmnt_hdl->init_flag = MNT_INIT_MAGIC;

    return 0;
}

static void __exit mnt_monitor_exit(void)
{
    return ;
}

module_init(mnt_monitor_init);
module_exit(mnt_monitor_exit);

/* Module information */
MODULE_AUTHOR(namo);
MODULE_DESCRIPTION(namo);
MODULE_LICENSE("GPL");

