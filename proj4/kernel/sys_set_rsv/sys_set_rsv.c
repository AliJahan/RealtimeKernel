#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>

#define MAX_RSV 20
#define CEIL_DIV(x, y) (x + y - 1) / y
/*
 * system call for setting reserver
 * <TEAM09/>
 */

SYSCALL_DEFINE4(set_rsv, pid_t, pid, struct timespec* , C, struct timespec*, T, int, cpuid){
    struct task_struct* task_ptr;
    struct task_struct* g;
    struct task_struct* p;
    int i;
    int j;
    int r_k;
    int r_kp1;
    int reserved_processes;
    typedef struct
    {
        int C;
        int T;
    } rsv_param_t;
    rsv_param_t *rsv_param_table;
    rsv_param_t swap_rsv_temp;

    pid_t intended_pid;

    // Checking validity of the inputs
    if((C->tv_nsec==0 && C->tv_sec==0) || (C->tv_nsec<0 || C->tv_sec<0))
        return -1;

    if((C->tv_sec > T->tv_sec) || ( (C->tv_sec == T->tv_sec) && (C->tv_nsec > T->tv_nsec)))
        return -1;

    if((T->tv_nsec==0 && T->tv_sec==0) || (T->tv_nsec<0 || T->tv_sec<0))
        return -1;

    if(pid<0)
        return -1;

    // Checking for pid=0
    intended_pid = pid;
    if(pid == 0)
        intended_pid = current->pid;

    rcu_read_lock();
    task_ptr = find_task_by_pid_ns(intended_pid, &init_pid_ns);
    rcu_read_unlock();

    if(task_ptr == NULL){
        return -1;
    }

    if(cpuid<0 || cpuid>3)
        return -1;


    // Setting C and T
    task_ptr->C = *C;
    task_ptr->T = *T;


    if (sched_setaffinity(intended_pid, cpumask_of(cpuid)) != 0)
        printk(KERN_INFO "<TEAM09>: Couldn't set init affinity to cpu (%d)\n", cpuid);

    rsv_param_table = (rsv_param_t *) kmalloc(MAX_RSV*sizeof(rsv_param_t), GFP_KERNEL);

    reserved_processes = 0;
    for_each_process_thread(g, p){
        if ((p->T.tv_nsec > 0) || (p->T.tv_sec > 0)){
            rsv_param_table[reserved_processes].C = (p->C.tv_sec)*1000 + (p->C.tv_nsec)/1000000;
            rsv_param_table[reserved_processes].T = (p->T.tv_sec)*1000 + (p->T.tv_nsec)/1000000;
            reserved_processes++;
        }
    }

    for (i = 0; i < reserved_processes-1; i++)
        for (j = 0; j < reserved_processes-i-1; j++)
            if (rsv_param_table[j].T > rsv_param_table[j+1].T){
                swap_rsv_temp = rsv_param_table[j];
                rsv_param_table[j] = rsv_param_table[j + 1];
                rsv_param_table[j + 1] = swap_rsv_temp;
            }



    for (i = 1; i < reserved_processes; i++){
        r_k = rsv_param_table[i].C;
        r_kp1 = rsv_param_table[i].C;
        do{
            r_k = r_kp1;
            r_kp1 = rsv_param_table[i].C;
            for (j = 0; j < i; j++){
              r_kp1 += CEIL_DIV(r_k,rsv_param_table[j].T)*rsv_param_table[j].C;
            }
            if(r_kp1 > rsv_param_table[i].T){
                kfree(rsv_param_table);
                task_ptr->C.tv_sec = 0;
                task_ptr->C.tv_nsec = 0;
                task_ptr->T.tv_sec = 0;
                task_ptr->T.tv_nsec = 0;
                printk(KERN_INFO "<TEAM09>: PID %d is not schedulable\n",(int)intended_pid);
                return -1;
            }

        }while(r_k != r_kp1);
    }

    printk(KERN_INFO "<TEAM09>: PID %d reserved C:%ld.%09ld\t T:%ld.%09ld \n",(int) intended_pid, task_ptr->C.tv_sec, task_ptr->C.tv_nsec, task_ptr->T.tv_sec, task_ptr->T.tv_nsec);

    kfree(rsv_param_table);
    return 0;
}
