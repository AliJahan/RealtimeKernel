#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#define set_rsv 397
int main(int argc, char* argv[]){

	int pid;

	/*printf("Enter pid\n");
	scanf("%d",&pid);

	printf("Result of calling by your PID\n");
	int ret = syscall(set_rsv,pid,3,4);
	if(ret<0)
		printf("NaN\n");
*/
	printf("Result of calling by process PID\n");
	int ret = syscall(set_rsv,getpid(),3,4);
	if(ret<0)
			printf("NaN\n");

	return 0;
}
