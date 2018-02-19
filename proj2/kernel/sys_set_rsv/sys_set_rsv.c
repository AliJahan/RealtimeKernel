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
#include <linux/mutex.h>

/*
 * system call for setting reserver
 * <TEAM09/>
 */
enum hrtimer_restart wake_upp(struct hrtimer *timer)
{
    struct timespec ts;
    long exec_time_us;
    long T_us=0.0;
    long util;
    printk(KERN_INFO "<TEAM09>: exec time(s):%ld.%ld\t C(s):%ld.%ld \n", timer->t->exec_time.tv_sec, timer->t->exec_time.tv_nsec, timer->t->C.tv_sec, timer->t->C.tv_nsec);
    // Checking if there is a task overrun
    if((timer->t->exec_time.tv_sec > timer->t->C.tv_sec) || ((timer->t->exec_time.tv_sec == timer->t->C.tv_sec) && (timer->t->exec_time.tv_nsec > timer->t->C.tv_nsec))){
        exec_time_us =  (timer->t->exec_time.tv_sec)*1000000 + (timer->t->exec_time.tv_nsec)/1000;
        T_us =  (timer->t->T.tv_sec)*1000000 + (timer->t->T.tv_nsec)/1000;
        util = (exec_time_us*100)/T_us;

        printk(KERN_INFO "<TEAM09>: Task %s (%d): budget overrun (util: %ld%%)\n", timer->t->comm, (int)timer->t->pid, util);
    }

    // RESET flag is set here to reset EXEC_TIME in scheduler
    atomic_set(&timer->t->reset,1);

    // waking up process
    wake_up_process(timer->t);

    // set interval for the next period
    ts = ktime_to_timespec(timer->base->get_time());
    hrtimer_forward_now(timer, timespec_to_ktime(timer->t->T));

    return HRTIMER_RESTART;
}

SYSCALL_DEFINE3(set_rsv, pid_t, pid, struct timespec* , C, struct timespec*, T){
    struct task_struct* task_ptr;
    pid_t tmp_pid;

    // Checking validity of the inputs
    if((C->tv_nsec==0 && C->tv_sec==0) || (C->tv_nsec<0 || C->tv_sec<0))
        return -1;

    if((T->tv_nsec==0 && T->tv_sec==0) || (T->tv_nsec<0 || T->tv_sec<0))
        return -1;

    if(pid<0)
        return -1;

    // Checking for pid=0
    tmp_pid = pid;
    if(pid == 0)
        tmp_pid = current->pid;

    rcu_read_lock();
    task_ptr = find_task_by_pid_ns(tmp_pid, &init_pid_ns);
    rcu_read_unlock();

    if(task_ptr == NULL){
        return -1;
    }
    // Setting C and T
    task_ptr->C = *C;
    task_ptr->T = *T;

    printk(KERN_INFO "<TEAM09>: PID %d reserved C:%d T:%d \n",(int) tmp_pid, (int)task_ptr->C.tv_nsec,(int)task_ptr->T.tv_nsec);

    // Initializing timer, setting callback function, and starting the timer
    hrtimer_init(&task_ptr->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
    task_ptr->timer.t = task_ptr;
    task_ptr->timer.function = &wake_upp;

    hrtimer_start(&task_ptr->timer, timespec_to_ktime(task_ptr->T), HRTIMER_MODE_REL_PINNED);
    task_ptr->time_stamp = ktime_to_timespec(ktime_get());
//    printk(KERN_INFO "<TEAM09> PID: %d started the timer \n",(int) tmp_pid);

    return 0;
}
