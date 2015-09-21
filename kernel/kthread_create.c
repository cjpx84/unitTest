
int kthread_fn(void *data)
{
	struct pt_regs *regs;
	regs = task_pt_regs(current);

     if(regs != NULL){
	    if(!user_mode(regs))
			printk("lijiazheng: kernel mode\n");
		else
			printk("lijiazheng: user mode\n");
     }
    printk("%s pid=%d\n",current->comm,current->pid);
	if(current->mm != NULL)
	    printk("lijiazheng:mm is not null\n");
	else
	    printk("lijiazheng:mm is null");

	while(!kthread_should_stop())
		{
		  
		  	
		  schedule();
		}
	return 0;
}
/*module init call */
struct task_struct *kthread = NULL;
	kthread = kthread_create(kthread_fn,NULL,"kthread_prism");

	if(kthread != NULL)
		wake_up_process(kthread);