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
	if(pid<0)//TODO: Check other scenarios too
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

	//TODO: timer stuff here
	//C
	t->C.tv_nsec = 0;
	t->C.tv_sec = 0;
	//T
	t->T.tv_nsec = 0;
	t->T.tv_sec = 0;

	printk(KERN_INFO "<TEAM09> Reservation for PID: %d has been canceled\n",(int) tmp_pid);
	return 0;
}
