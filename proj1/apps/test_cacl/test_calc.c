#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#define calc_num 397
int main(int argc, char* argv[]){
	
	int res;
	
	if(argc<4){
		printf("usage:\n ./test_calc arg1 arg2 operator\n operator:\n\t+\tadds arg1 and arg2\n\t-\t subtracts arg1 from arg2\n\t*\tmultiplies arg1 by arg2\n\t/\t divides arg1 to arg2\n");
		return 0;
	}
	
	int ret = syscall(calc_num,atoi(argv[1]),atoi(argv[3]),argv[2][0],&res);
	
	if(ret<0)
		printf("NaN\n");
	else
		printf("%d\n",res);
	
	return 0;
}
