#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <signal.h>
#define set_rsv 397
#define cancel_rsv 398
#define wait_until_next_period 399

struct reservation{
    long cs;
    long cns;
    long ts;
    long tns;
    long budget_ms;
};
void
signal_callback_handler(int signum)
{
   printf("Budget overrun signal received\n");
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

    printf("my PID:%d\n",getpid());
    int ret = syscall(set_rsv,0,&c,&t);


    int count = 1;
    struct timeval now, then, diff, t1, t2;
    while (1) {
        gettimeofday(&t1,NULL);
        while (1) {
            gettimeofday(&t2,NULL);
            timersub(&t2, &t1, &diff);

            if ((diff.tv_usec/1000+diff.tv_sec*1000) > input_args->budget_ms) break;
        }

        gettimeofday(&now,NULL);
        int ret = syscall(wait_until_next_period);
        gettimeofday(&then,NULL);
        if(ret<0)
            printf("Error Wait Until\n");
        timersub(&then, &now, &diff);
        printf("period %d: %ld. %ld\n", count++, diff.tv_sec, diff.tv_usec);

    }

}
int main(int argc, char* argv[]){

    if (argc < 4){
        printf("usage:\n\t ./set_rsv C T Comp\n");
        printf("\t C: Reserved Computation in milliseconds\n");
        printf("\t T: Reserved Period in milliseconds\n");
        printf("\t Comp: Computation time before calling wait_until_next_period in milliseconds\n");
        return -1;
    }
    struct reservation test;
    test.cs = atol(argv[1])/1000;
    test.cns = (atol(argv[1])%1000)*1000000;
    test.ts = atol(argv[2])/1000;
    test.tns = (atol(argv[2])%1000)*1000000;
    test.budget_ms = atol(argv[3]);

    //Assigning the process to one cpu core
    cpu_set_t my_set;
    CPU_ZERO(&my_set);
    CPU_SET(1, &my_set);
    sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

    pthread_t inc_x_thread;

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
