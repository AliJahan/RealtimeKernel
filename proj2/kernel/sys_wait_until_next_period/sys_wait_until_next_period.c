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
#include <linux/hrtimer.h>
/*
 * system call to suspend the task until the beginning of the next period of the task
 * <TEAM09/>
 */


SYSCALL_DEFINE0(wait_until_next_period){
    struct task_struct* task_ptr;
    pid_t tmp_pid;


    tmp_pid = current->pid;

    task_ptr = find_task_by_pid_ns(tmp_pid, &init_pid_ns);

    if(task_ptr == NULL){
        return -1;
    }

    if(task_ptr->C.tv_nsec == 0 && task_ptr->C.tv_sec == 0)
        return -1;

    if(task_ptr->T.tv_nsec == 0 && task_ptr->T.tv_sec == 0)
        return -1;

    __set_current_state(TASK_INTERRUPTIBLE);
    schedule();

    return 0;
}
