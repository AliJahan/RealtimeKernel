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
    struct siginfo info;

    // Checking if there is a task overrun
    if((timer->t->exec_time.tv_sec > timer->t->C.tv_sec) || ((timer->t->exec_time.tv_sec == timer->t->C.tv_sec) && (timer->t->exec_time.tv_nsec > timer->t->C.tv_nsec))){

        // converting execution and Period time in microseconds
        exec_time_us =  (timer->t->exec_time.tv_sec)*1000000 + (timer->t->exec_time.tv_nsec)/1000;
        T_us =  (timer->t->T.tv_sec)*1000000 + (timer->t->T.tv_nsec)/1000;

        // computing overrun utilization
        util = (exec_time_us*100)/T_us;

        //
        printk(KERN_INFO "<TEAM09>: exec time(s):%ld.%09ld\t C(s):%ld.%09ld \n", timer->t->exec_time.tv_sec, timer->t->exec_time.tv_nsec, timer->t->C.tv_sec, timer->t->C.tv_nsec);
        printk(KERN_INFO "<TEAM09>: Task %s (%d): budget overrun (util: %ld%%)\n", timer->t->comm, (int)timer->t->pid, util);

        // Sending SIGUSR1 signal to the thread
        memset(&info, 0, sizeof(struct siginfo));
        info.si_signo = SIGUSR1;
        if (send_sig_info(SIGUSR1, &info, timer->t) < 0) {
            printk(KERN_INFO "<TEAM09>: sending SIGUSR1 failed\n");
        }
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
    struct task_struct* g;
    struct task_struct* p;
    pid_t tmp_pid;


    struct sched_param param;//
    unsigned int min_prio = 100;//
    unsigned int max_prio = 0;//
    short increase_others = 1;



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


    for_each_process_thread(g, p){
        if ((p->T.tv_nsec > 0) || (p->T.tv_sec > 0)){
            if (((T->tv_sec == p->T.tv_sec) && (T->tv_nsec == p->T.tv_nsec))){
                min_prio = p->rt_priority;
                increase_others = 0;
            }
        }
    }
    if (increase_others == 1){
        for_each_process_thread(g, p){
            if ((p->T.tv_nsec > 0) || (p->T.tv_sec > 0))
            {
                if(max_prio < p->rt_priority)
                    max_prio = p->rt_priority;
                if ((T->tv_sec < p->T.tv_sec) || ((T->tv_sec == p->T.tv_sec) && (T->tv_nsec < p->T.tv_nsec))){
                    if(min_prio > p->rt_priority)
                        min_prio = p->rt_priority;
                    param.sched_priority = p->rt_priority + 1;
                    sched_setscheduler(p,SCHED_FIFO,&param);
                }
            }
        }
    }

    if(min_prio==100)
        param.sched_priority = max_prio + 1;
    else
        param.sched_priority = min_prio;

    sched_setscheduler(task_ptr,SCHED_FIFO,&param);

    // Setting C and T
    task_ptr->C = *C;
    task_ptr->T = *T;

    printk(KERN_INFO "<TEAM09>: PID %d reserved C:%ld.%09ld\t T:%ld.%09ld Priority:%d\n",(int) tmp_pid, task_ptr->C.tv_sec, task_ptr->C.tv_nsec, task_ptr->T.tv_sec, task_ptr->T.tv_nsec, param.sched_priority);

    // Initializing timer, setting callback function, and starting the timer
    hrtimer_init(&task_ptr->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_PINNED);
    task_ptr->timer.t = task_ptr;
    task_ptr->timer.function = &wake_upp;

    hrtimer_start(&task_ptr->timer, timespec_to_ktime(task_ptr->T), HRTIMER_MODE_REL_PINNED);
    task_ptr->time_stamp = ktime_to_timespec(ktime_get());
    //    printk(KERN_INFO "<TEAM09> PID: %d started the timer \n",(int) tmp_pid);

    return 0;
}
