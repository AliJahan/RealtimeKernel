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
 * system call for setting reserver
 * <TEAM09/>
 */

SYSCALL_DEFINE3(set_rsv, pid_t, pid, struct timespec* , C, struct timespec*, T){
	struct task_struct* t;
	pid_t tmp_pid;
	//Check validity of the inputs
	if(C->tv_nsec<=0 && C->tv_sec<=0)
		return -1;

	if(T->tv_nsec<=0 && T->tv_sec<=0)
		return -1;

	if(pid<0)//TODO: Check other scenarios too
		return -1;

	//check for pid=0
	tmp_pid = pid;
	if(pid == 0)
		tmp_pid = current->pid;

	t = find_task_by_pid_ns(tmp_pid, &init_pid_ns);

	if(t == NULL){
		return -1;
	}
	//set C and T
	t->C.tv_nsec = (C->tv_nsec!=0) ? (C->tv_nsec) : (C->tv_sec*1000000000);
	t->C.tv_sec = (C->tv_sec!=0) ? (C->tv_sec) : (C->tv_nsec/1000000000);
	t->T.tv_nsec = (T->tv_nsec!=0) ? (T->tv_nsec) : (T->tv_sec*1000000000);
	t->T.tv_sec = (T->tv_sec!=0) ? (T->tv_sec) : (T->tv_nsec/1000000000);
	//TODO: timer stuff
	printk(KERN_INFO "<TEAM09> PID: %d has reserved C:%d T:%d \n",(int) tmp_pid, (int)t->C.tv_nsec,(int)t->T.tv_nsec);
	return 0;
}
