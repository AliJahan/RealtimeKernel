#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <asm/syscall.h>



#define MAX_RSV 20
#define BIN_SIZE 10000
#define CEIL_DIV(x, y) (x + y - 1) / y
/*
 * system call for setting reserver
 * <TEAM09/>
 */
char partition_policy[10] = "BF";
EXPORT_SYMBOL(partition_policy);

SYSCALL_DEFINE4(set_rsv, pid_t, pid, struct timespec* , C, struct timespec*, T, int, cpuid){
    struct task_struct* task_ptr;
    struct task_struct* g;
    struct task_struct* p;
    int i;
    int j;
    int k;
    int r_k;
    int r_kp1;
    int reserved_processes[NR_CPUS];
    int cpus_utilization[NR_CPUS];
    short rpt_passed[NR_CPUS];
    int cpuid_selected;
    int max_cpus_utilization;
    int min_cpus_utilization;

    //data structure to store processes' properties (C,T)
    typedef struct
    {
        int C;
        int T;
    } rsv_param_t;
    rsv_param_t rsv_param_table[NR_CPUS][MAX_RSV];
    rsv_param_t swap_rsv_temp;//temp for swap in sorting

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

    if(cpuid<-1 || cpuid>3)
        return -1;

    // Initializations
    for(i = 0; i<NR_CPUS; i++){
        reserved_processes[i] = 0;
        cpus_utilization[i] = 0;
        rpt_passed[i] = 1;
    }

    // Extract all processes' info with C&T
    for_each_process_thread(g, p){
        if ((p->T.tv_nsec > 0) || (p->T.tv_sec > 0)){
            rsv_param_table[p->cpu_no][reserved_processes[p->cpu_no]].C = (p->C.tv_sec)*1000 + (p->C.tv_nsec)/1000000;
            rsv_param_table[p->cpu_no][reserved_processes[p->cpu_no]].T = (p->T.tv_sec)*1000 + (p->T.tv_nsec)/1000000;
            cpus_utilization[p->cpu_no] += ((rsv_param_table[p->cpu_no][reserved_processes[p->cpu_no]].C) * BIN_SIZE)/ (rsv_param_table[p->cpu_no][reserved_processes[p->cpu_no]].T);
            reserved_processes[p->cpu_no]++;
        }
    }

    //insert current process to all rsv_param_tables
    for(i = 0; i<NR_CPUS; i++){
        rsv_param_table[i][reserved_processes[i]].C = (C->tv_sec)*1000 + (C->tv_nsec)/1000000;
        rsv_param_table[i][reserved_processes[i]].T = (T->tv_sec)*1000 + (T->tv_nsec)/1000000;
        cpus_utilization[i] += ((rsv_param_table[i][reserved_processes[i]].C) * BIN_SIZE)/ (rsv_param_table[i][reserved_processes[i]].T);
        reserved_processes[i]++;
    }

    //sort
    for(k = 0 ;k < NR_CPUS ; k++)
        for (i = 0; i < reserved_processes[k]-1; i++)
            for (j = 0; j < reserved_processes[k]-i-1; j++)
                if (rsv_param_table[k][j].T > rsv_param_table[k][j+1].T){
                    swap_rsv_temp = rsv_param_table[k][j];
                    rsv_param_table[k][j] = rsv_param_table[k][j + 1];
                    rsv_param_table[k][j + 1] = swap_rsv_temp;
                }

    // Response time test for all CPUs (bins)
    for(k = 0 ;k < NR_CPUS ; k++){
        if(cpus_utilization[k] <= BIN_SIZE)
            for (i = 1; i < reserved_processes[k]; i++){
                r_k = rsv_param_table[k][i].C;
                r_kp1 = rsv_param_table[k][i].C;
                do{
                    r_k = r_kp1;
                    r_kp1 = rsv_param_table[k][i].C;
                    for (j = 0; j < i; j++){
                        r_kp1 += CEIL_DIV(r_k,rsv_param_table[k][j].T)*rsv_param_table[k][j].C;
                    }
                    if(r_kp1 > rsv_param_table[k][i].T){
                        printk(KERN_INFO "<TEAM09>: PID %d on core %d is not schedulable\n",(int)intended_pid, k);
                        rpt_passed[k] = 0;
                    }

                }while(r_k != r_kp1);
            }
        else
            rpt_passed[k] = 0;
    }


    cpuid_selected = cpuid;
    max_cpus_utilization = 0;
    min_cpus_utilization = BIN_SIZE + 1;
    //user requested CPU
    if(cpuid >-1){
        if (rpt_passed[cpuid] == 0)
            return -1;
    }
    //Assign CPU based on partitioning policy
    else{

        if (strcmp(partition_policy,"WF") == 0){
            for(k = 0 ;k < NR_CPUS ; k++)
                if(rpt_passed[k] == 1 && cpus_utilization[k] < min_cpus_utilization){
                    min_cpus_utilization = cpus_utilization[k];
                    cpuid_selected = k;
                }
        }
        else if (strcmp(partition_policy,"FF") == 0){
            for(k = 0 ;k < NR_CPUS ; k++)
                if(rpt_passed[k] == 1){
                    cpuid_selected = k;
                    break;
                }
        }
        else{
            for(k = 0 ;k < NR_CPUS ; k++)
                if(rpt_passed[k] == 1 && cpus_utilization[k] > max_cpus_utilization){
                    max_cpus_utilization = cpus_utilization[k];
                    cpuid_selected = k;
                }
        }

    }

    if (cpuid_selected == -1)
        return -1;

    // Setting C and T
    task_ptr->C = *C;
    task_ptr->T = *T;

    //set CPU for process
    if (sched_setaffinity(intended_pid, cpumask_of(cpuid_selected)) != 0){
        printk(KERN_INFO "<TEAM09>: Couldn't set init affinity to cpu (%d)\n", cpuid_selected);
        return -1;
    }

    task_ptr->cpu_no = cpuid_selected;

    printk(KERN_INFO "<TEAM09>: PID %d reserved C:%ld.%09ld\t T:%ld.%09ld on CPU %d with Utilization: %d Partitioning policy: %s\n",(int) intended_pid, task_ptr->C.tv_sec, task_ptr->C.tv_nsec, task_ptr->T.tv_sec, task_ptr->T.tv_nsec, cpuid_selected, cpus_utilization[cpuid_selected], partition_policy);

    return 0;
}
