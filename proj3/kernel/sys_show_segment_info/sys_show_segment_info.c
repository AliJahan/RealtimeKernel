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
 * system call to print the start and end addresses of the code, data, and heap segments of a process
 * <TEAM09/>
 */

SYSCALL_DEFINE1(show_segment_info, pid_t, pid){
	struct task_struct* t;
	pid_t tmp_pid;


	if(pid<0)
		return -1;

	//check for pid=0
	tmp_pid = pid;
	if(pid == 0)
		tmp_pid = current->pid;

	t = find_task_by_pid_ns(tmp_pid, &init_pid_ns);
	if(t == NULL)
		return -1;

    printk(KERN_INFO "<TEAM09>: [Memory segment addresses of process %d]\n",(int) tmp_pid);
    printk(KERN_INFO "<TEAM09>: %08x - %08x: code segment\n", (unsigned int)t->mm->start_code, (unsigned int)t->mm->end_code);
    printk(KERN_INFO "<TEAM09>: %08x - %08x: data segment\n", (unsigned int)t->mm->start_data, (unsigned int)t->mm->end_data);
    printk(KERN_INFO "<TEAM09>: %08x - %08x: heap segment\n", (unsigned int)t->mm->start_brk, (unsigned int)t->mm->brk);

//	printk(KERN_INFO "<TEAM09>: Reservation for PID %d has been canceled in CANCEL_RSV\n",(int) tmp_pid);
	return 0;
}
