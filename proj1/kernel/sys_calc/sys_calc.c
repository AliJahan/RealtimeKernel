#include <linux/export.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/ipc.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
/*
 * system call for a simple calculator
 * <TEAM09/>
 */
SYSCALL_DEFINE4(calc, int, arg1, int, arg2, char, op, int*, result)
{
  int ret=0;
  if(op=='/' && arg2==0)
	return -1;

  switch(op)
  {
	case '+':
		*result = arg1 + arg2;
		break;
	case '-':
		*result = arg1 - arg2;
		break;
	case '/':
		*result = arg1 / arg2;
  		break;
	case '*':
		*result = arg1 * arg2;
  		break;
  	default:
		ret = -1;
  		break;            
  }
  return ret;
}
