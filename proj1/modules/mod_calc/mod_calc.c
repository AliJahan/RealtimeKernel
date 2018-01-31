#include <linux/init.h>
#include <asm/syscall.h>
#include <linux/module.h>

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("team09");    
MODULE_DESCRIPTION("This module hijacks \"calc\" system call");

extern unsigned long sys_call_table[];

asmlinkage long (*original_sys_calc)(int,int,char,int*);

asmlinkage long fake_calc_function(int arg1, int arg2, char op, int* result)
{
    *result = arg1 % arg2;
    return 0;
}

static int __init mod_calc_init(void){

   printk(KERN_INFO "system call \"calc\" is hijacked (TEAM09)\n");
   
   original_sys_calc = (void*)sys_call_table[__NR_calc];

   sys_call_table[__NR_calc]=(unsigned long)fake_calc_function;

   return 0;
}


static void __exit mod_calc_exit(void)
{
   printk(KERN_INFO "the hijacked system call \"calc\" was freed (TEAM09)\n");
   sys_call_table[__NR_calc]=(unsigned long)original_sys_calc;
}

module_exit(mod_calc_exit);
module_init(mod_calc_init);
