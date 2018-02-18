#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#define set_rsv 397
#define cancel_rsv 398
int main(int argc, char* argv[]){

	FILE *fp1;
	int pid;
	struct timespec c;
	c.tv_nsec = 100;
	c.tv_sec = 1;

	struct timespec t;
	t.tv_nsec = 200;
	t.tv_sec = 2;
	int i;
	printf("my PID:%d\n",getpid());
	for(i=0;i<5;i++){
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

	fp1= fopen("/dev/rsvdev", "r");
	if (fp1 == NULL)
	{
	    puts("Error while opening file");

	}
	printf("-------------------------\n");
//	for(i=0;i<100;i++){
//		printf("Trying to cancel PID:%d reservation",i);
//		int ret = syscall(cancel_rsv,i);
//		if(ret<0)
//			printf(" -> can not cancel reservation");
//		printf("\n");
//	}
	int f;
	printf("-------------------------\n");
	scanf("%d",&f);
    fclose(fp1);
	return 0;
}
