/*
#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel
#include <linux/unistd.h>
#include <asm/syscall.h>
 */

#include <linux/init.h>
#include <asm/syscall.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/page.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("team09");    
MODULE_DESCRIPTION("This module hijacks calc system call");

//extern const unsigned long *sys_call_table[];
//EXPORT_SYMBOL(sys_call_table);

asmlinkage long (*original_sys_calc)(int,int,char,int*);

asmlinkage long fake_calc_function(int arg1, int arg2, char op, int* result)
{
    *result = arg1 % arg2;
    return 0;
}


static void disable_page_protection(void) 
{
  unsigned long value;
  asm volatile("mov %%cr0, %0" : "=r" (value));

  if(!(value & 0x00010000))
    return;

  asm volatile("mov %0, %%cr0" : : "r" (value & ~0x00010000));
}

static void enable_page_protection(void) 
{
  unsigned long value;
  asm volatile("mov %%cr0, %0" : "=r" (value));

  if((value & 0x00010000))
    return;

  asm volatile("mov %0, %%cr0" : : "r" (value | 0x00010000));
}

static unsigned long **aquire_sys_call_table(void)
{
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;

	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;

		if (sct[__NR_calc] == (unsigned long *) sys_calc) 
			return sct;

		offset += sizeof(void *);
	}
	
	return NULL;
}

static int __init mod_calc_init(void){
   if(!(sys_call_table = aquire_sys_call_table()))
    return -1;
   /*store reference to the original sys_exit*/
   original_sys_calc=sys_call_table[__NR_calc];

   /*manipulate sys_call_table to call our
    *fake exit function instead
    *of sys_exit*/
   sys_call_table[__NR_calc]=fake_calc_function;

   return 0;
}


static void __exit mod_calc_exit(void)
{
     sys_call_table[__NR_calc]=original_sys_calc;
}

module_exit(mod_calc_exit);
module_init(mod_calc_init);
