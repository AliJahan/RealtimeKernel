#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>

#define set_rsv 397
#define cancel_rsv 398
#define wait_until_next_period 399

void *set_res(void* input_t)
{
    FILE *fp1;
    int f;
    cpu_set_t cpuset;

    //the CPU we want to use
    int cpu = 1;

    CPU_ZERO(&cpuset);       //clears the cpuset
    CPU_SET( cpu , &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset),
            &cpuset) <0) {
        perror("pthread_setaffinity_np");
    }


    struct timespec c;
    c.tv_nsec = 8000;
    c.tv_sec = 0;

    struct timespec t;
    t.tv_nsec = 2000000000;
    t.tv_sec = 2;
    int i;
    printf("my PID:%d\n",getpid());
    for(i=0;i<1;i++){
        printf("PID:%d ",i);
        c.tv_nsec +=i;
        c.tv_sec +=i;
        t.tv_nsec +=i;
        t.tv_sec +=i;
        int ret = syscall(set_rsv,i,&c,&t);
        if(ret<0)
            printf("NaN for %d",i);
        printf("\n");
    }

    //  fp1= fopen("/dev/rsvdev", "r");
    //  if (fp1 == NULL)
    //  {
    //      puts("Error while opening file");
    //
    //  }
    //  printf("-------------------------\n");
    //  for(i=0;i<100;i++){
    //      printf("Trying to cancel PID:%d reservation",i);
    //      int ret = syscall(cancel_rsv,i);
    //      if(ret<0)
    //          printf(" -> can not cancel reservation");
    //      printf("\n");
    //  }
    //    printf("-------------------------\n");
    //    scanf("%d",&f);
    //    printf("Process:%d\n",f);

    //    fclose(fp1);

    //    scanf("%d",&f);



    int count = 1;
    //    struct timespec t_0, t_now, t_diff;
    struct timeval now, then, diff;
    gettimeofday(&now, NULL);

    while (1) {
       for(volatile int i=0;i<10;i++);
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

    int test;


    pthread_t inc_x_thread;

    /* create a second thread which executes inc_x(&x) */
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
