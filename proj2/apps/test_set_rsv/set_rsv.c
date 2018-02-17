#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#define set_rsv 397
int main(int argc, char* argv[]){

	int res;
	if(argc<4){
		printf("usage:\n ./test_calc arg1 operator arg2\n operator:\n\t+\tadds arg1 and arg2\n\t-\t subtracts arg1 from arg2\n\t*\tmultiplies arg1 by arg2\n\t/\t divides arg1 to arg2\n");
		return 0;
	}

	int pid;

	printf("Enter pid\n");
	scanf("%d",&pid);

	printf("Result of calling by your PID\n");
	int ret = syscall(set_rsv,pid,3,4);
	if(ret<0)
		printf("NaN\n");

	printf("Result of calling by process PID\n");
	ret = syscall(set_rsv,getpid(),3,4);
	if(ret<0)
			printf("NaN\n");

	return 0;
}
