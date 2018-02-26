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

SYSCALL_DEFINE1(show_vm_areas, pid_t, pid){
	struct task_struct* t;
	pid_t tmp_pid;
    struct vm_area_struct *current_vma;
    char locked[4];

	if(pid<0)
		return -1;

	//check for pid=0
	tmp_pid = pid;
	if(pid == 0)
		tmp_pid = current->pid;

	t = find_task_by_pid_ns(tmp_pid, &init_pid_ns);
	if(t == NULL)
		return -1;

	current_vma = t->mm->mmap;
    printk(KERN_INFO "<TEAM09>: [Memory-mapped areas of process %d]\n",(int) tmp_pid);

	//VM_LOCKED 0x00002000
	while(current_vma != NULL){
	    strcpy(locked, " ");
	    if(current_vma->vm_flags & VM_LOCKED)
	        strcpy(locked, "[L]");
	    printk(KERN_INFO "<TEAM09>: %08x - %08x: %lu bytes %s\n", (unsigned int)current_vma->vm_start, (unsigned int)current_vma->vm_end, (current_vma->vm_end - current_vma->vm_start), locked);
	    current_vma = current_vma->vm_next;
	}
//	printk(KERN_INFO "<TEAM09>: Reservation for PID %d has been canceled in CANCEL_RSV\n",(int) tmp_pid);
	return 0;
}
