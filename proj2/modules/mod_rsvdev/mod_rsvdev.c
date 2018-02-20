#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/uaccess.h>        // Required for the copy to user function
#define  DEVICE_NAME "rsvdev"     ///< The device will appear at /dev/rsvdev using this value
#define  CLASS_NAME  "TEAM09"     ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("TEAM09");
MODULE_DESCRIPTION("This module outputs a list of tasks having active reservations");

static int    majorNumber;
static char   header[25] = "pid\t tgid\t prio\t name\n";
static short    numberOpens = 0;
static struct class*  rsvdevClass  = NULL;
static struct device* rsvdevDevice = NULL;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);


static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .release = dev_release,
};

static int __init rsvdev_init(void){

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
        printk(KERN_ALERT "TEAM09: failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "TEAM09: registered correctly with major number %d\n", majorNumber);

    // Register the device class
    rsvdevClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(rsvdevClass)){                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "TEAM09: Failed to register device class\n");
        return PTR_ERR(rsvdevClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "TEAM09: device class registered correctly\n");

    // Register the device driver
    rsvdevDevice = device_create(rsvdevClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(rsvdevDevice)){               // Clean up if there is an error
        class_destroy(rsvdevClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "TEAM09: Failed to create the device\n");
        return PTR_ERR(rsvdevDevice);
    }
    printk(KERN_INFO "TEAM09: device class created correctly\n"); // Made it! device was initialized
    return 0;
}


static void __exit rsvdev_exit(void){
    device_destroy(rsvdevClass, MKDEV(majorNumber, 0));     // remove the device
    class_unregister(rsvdevClass);                          // unregister the device class
    class_destroy(rsvdevClass);                             // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
    printk(KERN_INFO "TEAM09: Goodbye from the LKM!\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
    if(numberOpens>0)
        return EBUSY;
    numberOpens++;
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){

    static char message[102400];
    int  size_of_message;
    struct task_struct *g, *p;
    char tempString[100];
    strcpy(message,"");
    strcat(message, header);
    size_of_message = 0;
    for_each_process_thread(g, p) {
        if ((p->C.tv_nsec !=0 ) || (p->C.tv_sec !=0 )){
            sprintf(tempString, "%d\t %d\t %d\t %s\n", p->pid, p->tgid, p->prio, p->comm);
            strcat(message, tempString);
        }

    }

    size_of_message = strlen(message);

    if( *offset >= size_of_message )
        return 0;

    if( *offset + len > size_of_message )
        len = size_of_message - *offset;

    if( copy_to_user(buffer, message + *offset, len) != 0 )
        return -EFAULT;

    *offset += len;
    return len;
}

static int dev_release(struct inode *inodep, struct file *filep){
    numberOpens--;
    return 0;
}

module_init(rsvdev_init);
module_exit(rsvdev_exit);
