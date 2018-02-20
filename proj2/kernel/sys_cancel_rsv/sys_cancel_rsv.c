#include <linux/export.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <asm/syscall.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/ipc.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
/*
 * system call for a canceling reserve
 * <TEAM09/>
 */

SYSCALL_DEFINE1(cancel_rsv, pid_t, pid){
	struct task_struct* t;
	pid_t tmp_pid;


    struct task_struct* g;
    struct task_struct* p;

    struct sched_param param;
    short decrease_others = 1;

	if(pid<0)
		return -1;

	//check for pid=0
	tmp_pid = pid;
	if(pid == 0)
		tmp_pid = current->pid;

	t = find_task_by_pid_ns(tmp_pid, &init_pid_ns);
	if(t == NULL)
		return -1;

	if(t->C.tv_nsec == 0 && t->C.tv_sec == 0)
		return -1;

	if(t->T.tv_nsec == 0 && t->T.tv_sec == 0)
		return -1;


	//Reset Priority
	for_each_process_thread(g, p){
	    if ((p != t) && ((p->T.tv_nsec > 0) || (p->T.tv_sec > 0))){
	        if (((t->T.tv_sec == p->T.tv_sec) && (t->T.tv_nsec == p->T.tv_nsec)))
	            decrease_others = 0;
	    }
	}

	if (decrease_others==1){
	    for_each_process_thread(g, p){
	        if ((p->T.tv_nsec > 0) || (p->T.tv_sec > 0))
	        {
	            if(t->rt_priority < p->rt_priority){
	                param.sched_priority = p->rt_priority - 1;
	                sched_setscheduler(p,SCHED_FIFO,&param);
	            }
	        }
	    }
	}
	//set C
	t->C.tv_nsec = 0;
	t->C.tv_sec = 0;
	//set T
	t->T.tv_nsec = 0;
	t->T.tv_sec = 0;
	//Cancel timer
    hrtimer_cancel(&t->timer);

	printk(KERN_INFO "<TEAM09>: Reservation for PID %d has been canceled in CANCEL_RSV\n",(int) tmp_pid);
	return 0;
}
