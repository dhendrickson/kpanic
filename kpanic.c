/*
 * Copyright (c) 2011 Dusty Hendrickson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Dusty Hendrickson");
MODULE_DESCRIPTION("Presents procfs entry to force a kernel panic");

#define PROCFS_PATH "kpanic"
#define PROCFS_BUFFER_MAX_SIZE 1024

static struct proc_dir_entry* procfsEntry;

static int kpanic_write(struct file* file, const char* buffer, unsigned long count, void* data)
{
    char procfsBuffer[PROCFS_BUFFER_MAX_SIZE];
    unsigned long procfsBufferSize;
    
    // Limit size of buffer copy
    procfsBufferSize = count;
    if(procfsBufferSize > PROCFS_BUFFER_MAX_SIZE)
    {
        procfsBufferSize = PROCFS_BUFFER_MAX_SIZE;
    }

    // Copy buffer from user space to kernel space
    if(copy_from_user(procfsBuffer, buffer, procfsBufferSize))
    {
        return -EFAULT;
    }
    procfsBuffer[procfsBufferSize] = '\0';

    //printk(KERN_INFO "Write: %s", procfsBuffer);
    panic("Forcing panic via kpanic");
    
    return procfsBufferSize;
}

static int __init kpanic_init(void)
{
    printk(KERN_INFO "Loading kpanic module\n");
   
    // Create procfs entry
    procfsEntry = create_proc_entry(PROCFS_PATH, 0400, NULL);
    if(procfsEntry == NULL)
    {
        remove_proc_entry(PROCFS_PATH, NULL);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_PATH);
        return -ENOMEM;
    }

    // Set procfs entry parameters
    procfsEntry->write_proc = kpanic_write;
    procfsEntry->mode = S_IFREG | S_IRUGO;
    procfsEntry->uid = 0;
    procfsEntry->gid = 0;
    procfsEntry->size = 0;

    return 0;
}

static void __exit kpanic_exit(void)
{
    printk(KERN_INFO "Unloading kpanic module\n");

    remove_proc_entry(PROCFS_PATH, NULL);

    return;
}

module_init(kpanic_init);
module_exit(kpanic_exit);

