#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#define set_rsv 397
int main(int argc, char* argv[]){

	int pid;
	struct timespec c;
	c.tv_nsec = 300;
	c.tv_sec = 3;

	struct timespec t;
	t.tv_nsec = 500;
	t.tv_sec = 5;

	printf("Enter pid\n");
	scanf("%d",&pid);

	printf("Result of calling by your PID\n");
	int ret = syscall(set_rsv,pid,&c,&t);
	if(ret<0)
		printf("NaN\n");

	printf("Result of calling by process PID\n");
	ret = syscall(set_rsv,getpid(),&c,&t);
	if(ret<0)
			printf("NaN\n");

	return 0;
}
