#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define BILLION 1000000000L
#define show_segment_info 400
#define show_vm_areas 401
/*
 * This application test system calls
 * Prints memory address of a process in kernel log (calls show_segment_info system call)
 * Prints memory-mapped areas of a process in kernel log (calls show_vm_areas system call)
 * If the PID argument is not provided, zero would be passed to the system call as pid (the system call prints the information for the current process)
 */
int main(int argc, char* argv[]){

    int pid = 0;
    char *buf1 = NULL;
    char *buf2 = NULL;

    if (argc != 3 && argc != 2){
        printf("usage:\n\t ./test_app SIZE [PID]\n");
        printf("\t SIZE: Size to be allocated and locked in bytes\n");
        printf("\t PID: pid of the process, default: current process\n");
        return -1;
    }

    if (argc == 3)
        pid = atoi(argv[2]);
    else{
        int n = atoi(argv[1]);
        buf1 = (char *)malloc(n);

        if(buf1 == NULL){
            printf("Error allocating Memory\n");
            return -1;
        }

        buf2 = (char *)malloc(n);
        if(buf2 == NULL){
            printf("Error allocating Memory\n");
            return -1;
        }

        if (mlock(buf1, n)) {
            free(buf1);
            return -1;
        }
        for (int i = 0; i < n; i += 4096) /* write one byte in each page (4096 bytes) */
            buf1[i] = 1;
        for (int i = 0; i < n; i += 4096) /* write one byte in each page (4096 bytes) */
            buf2[i] = 1;

    }

    syscall(show_segment_info, pid);
    syscall(show_vm_areas, pid);
    if(buf1)
        free(buf1);
    if(buf2)
        free(buf2);
    return 0;
}
