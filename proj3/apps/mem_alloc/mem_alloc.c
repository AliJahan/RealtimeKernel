#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000L
int main(int argc, char* argv[]){


    if (argc != 2){
        printf("usage:\n\t ./mem_alloc SIZE\n");
        printf("\t SIZE: Size to be allocated in bytes\n");
        return -1;
    }

    long long unsigned int diff;
    struct timespec start, end;
    int n = atoi(argv[1]);
    char *buf = (char *)malloc(n);
    if(buf == NULL){
        printf("Error allocating Memory\n");
        return -1;
    }


    /* measure monotonic time */
    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */
    for (int i = 0; i < n; i += 4096) /* write one byte in each page (4096 bytes) */
        buf[i] = 1;
    clock_gettime(CLOCK_REALTIME, &end);   /* mark the end time */

    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

    printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);
    free(buf);
    return 0;
}
