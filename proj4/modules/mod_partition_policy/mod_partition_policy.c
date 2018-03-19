#include <linux/module.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("TEAM09");
MODULE_DESCRIPTION("This module Provide a virtual sysfs file at /sys/ee255/partition_policy for specifying and querying the current partitioning heuristic");

// shared variable for partitioning policy
extern char partition_policy[10];


static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

struct kobject *kernel_obj_ref;
struct kobj_attribute sysfs_attrib = __ATTR(partition_policy, 0660, sysfs_show, sysfs_store);

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    printk(KERN_INFO "<TEAM09>: Read from sysfs\n");
    return sprintf(buf, "%s\n", partition_policy);
}


static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    if ((strcmp(buf,"BF\n") == 0) || (strcmp(buf,"BF") == 0) )
        strcpy(partition_policy, "BF");
    else if ((strcmp(buf,"WF\n") == 0) || (strcmp(buf,"WF") == 0) )
        strcpy(partition_policy, "WF");
    else if ((strcmp(buf,"FF\n") == 0) || (strcmp(buf,"FF") == 0) )
        strcpy(partition_policy, "FF");
    else{
        printk(KERN_INFO "<TEAM09>: Invalid input. Only BF, FF, and WF are acceptable inputs\n");
        return -EINVAL;
    }

    printk(KERN_INFO "<TEAM09>: Write to sysfs. Partitioning policy changed to %s\n", partition_policy);
    return count;
}


static int __init device_init(void)
{
    kernel_obj_ref = kobject_create_and_add("ee255",NULL);

    if(sysfs_create_file(kernel_obj_ref,&sysfs_attrib.attr)){
        printk(KERN_INFO"<TEAM09>: Cannot create sysfs file\n");
        kobject_put(kernel_obj_ref);
        sysfs_remove_file(kernel_kobj, &sysfs_attrib.attr);
        return -1;
    }
    printk(KERN_INFO "<TEAM09>: Sysfs file has been created\n");
    return 0;
}

void __exit device_exit(void)
{
    strcpy(partition_policy, "BF"); // return to default value (Best Fit policy)
    kobject_put(kernel_obj_ref);
    sysfs_remove_file(kernel_kobj, &sysfs_attrib.attr);
    printk(KERN_INFO "<TEAM09>: Sysfs file has been removed\n");
}

module_init(device_init);
module_exit(device_exit);

