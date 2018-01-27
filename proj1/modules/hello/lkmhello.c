
#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel
 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("team09");    
MODULE_DESCRIPTION("A simple Linux loadable kernel module");  
 
static int __init lkmhello_init(void){
   printk(KERN_INFO "Hello world! team09 from kernel space\n");
   return 0;
}


static void __exit lkmhello_exit(void)
{
	printk(KERN_INFO "Goodbye, team09\n");
}

module_exit(lkmhello_exit);
module_init(lkmhello_init);
