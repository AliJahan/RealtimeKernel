#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>

#define set_rsv 397
#define cancel_rsv 398
#define wait_until_next_period 399

struct reservation{
    long pid;
    long cs;
    long cns;
    long ts;
    long tns;
    long cpuid;
};
void
signal_callback_handler(int signum)
{
//   printf("Budget overrun signal received\n");
}
void *set_res(void* input_t)
{
    signal(SIGUSR1, signal_callback_handler);
    int f;
    struct reservation* input_args = (struct reservation*)input_t;

    struct timespec c;
    c.tv_nsec = input_args->cns;
    c.tv_sec = input_args->cs;

    struct timespec t;
    t.tv_nsec = input_args->tns;
    t.tv_sec = input_args->ts;

    printf("my PID:%ld\n",syscall(SYS_gettid));
    int ret = syscall(set_rsv,input_args->pid, &c, &t, input_args->cpuid);
    if(ret<0)
        printf("PID:%ld could not reserve requested resource\n", syscall(SYS_gettid));

    int count = 1;
    struct timeval now, then, diff, t1, t2;
    while(1);
    getchar();
}
int main(int argc, char* argv[]){

    if (argc != 5){
        printf("usage:\n\t ./set_rsv <PID> <execution time C in ms> <period T in ms> <CPU core ID>\n");
//        printf("\t C: Reserved Computation in milliseconds\n");
//        printf("\t T: Reserved Period in milliseconds\n");
//        printf("\t Comp: Computation time before calling wait_until_next_period in milliseconds\n");
        return -1;
    }
    struct reservation test;
    test.pid = atol(argv[1]);
    test.cs = atol(argv[2])/1000;
    test.cns = (atol(argv[2])%1000)*1000000;
    test.ts = atol(argv[3])/1000;
    test.tns = (atol(argv[3])%1000)*1000000;
    test.cpuid = atol(argv[4]);

    pthread_t inc_x_thread;
    printf("parent PID:%d\n",getpid());
    /* create a thread to set reservation for*/
    if(pthread_create(&inc_x_thread, NULL, set_res, (void*)&test)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if(pthread_join(inc_x_thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }
    return 0;
}
