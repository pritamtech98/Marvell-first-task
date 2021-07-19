#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/kdev_t.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRIT");
MODULE_DESCRIPTION("The chr device");
#define MINOR_NO 0
#define MEM_SIZE 1024

dev_t dev=0;
struct cdev *my_cdev;
char *kbuff;

static int my_open(struct inode *inode, struct file *f){
  if((kbuff = kmalloc(MEM_SIZE, GFP_KERNEL)) == 0){
    printk(KERN_INFO"error generating mem for device\n");
    return -1;
  }else{
    printk(KERN_INFO"dev opened success\n");
    int i;
    for(i=0; i< MEM_SIZE; i++)
      kbuff[i] = '\0';
    return 0;
  }
}


static int my_release(struct inode *inode, struct file *f){
  kfree(kbuff);
  printk(KERN_INFO"dev closed success\n");
  return 0;
}


static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *offp){
  // copy_to_user(buf, kbuff, MEM_SIZE);
  // printk(KERN_INFO"Data copied to user space \n");
  // return MEM_SIZE;
  int nbytes, maxbytes, bytes_to_do;
  maxbytes = MEM_SIZE - *offp;
  bytes_to_do = (maxbytes >= len)?len:maxbytes;
  nbytes = bytes_to_do - copy_to_user(buf, kbuff+*offp, bytes_to_do);
  *offp += nbytes;
  printk(KERN_INFO"Data send to applicatiom: %s | nbytes=%d\n", kbuff, nbytes);
  return nbytes;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *offp){
  // copy_from_user(kbuff, buf, len);
  // printk(KERN_INFO"Data written from user space to kernel space \n");
  // return len;
  int nbytes;
  nbytes = len - copy_from_user(kbuff+*offp, buf, len);
  *offp += nbytes;
  printk(KERN_INFO"Data rec from applicatiom: %s | nbytes=%d\n", kbuff, nbytes);
  return nbytes;
}

struct file_operations my_dev_ops = {
  .owner = THIS_MODULE,
  .read = my_read,
  .write = my_write,
  .open = my_open,
  .release = my_release,
};

static int __init chr_dev_init(void){
  int val, major_no, minor_no;
  if((val = alloc_chrdev_region(&dev, MINOR_NO, 1, "my_dev")) < 0){
    printk(KERN_INFO"failed to register the device numbers\n");
    return -1;
  }
  major_no = MAJOR(dev);
  minor_no = MINOR(dev);

  my_cdev = cdev_alloc();
  my_cdev->owner = THIS_MODULE;
  my_cdev->ops = &my_dev_ops;
  if((val = cdev_add(my_cdev,dev, 1)) < 0){
    printk(KERN_INFO"failed to add the device driver\n");
    return -1;
  }
  printk(KERN_INFO"Dev added success with major no as : {%d} and minor no as : {%d}\n", major_no, minor_no);
  return 0;
}


static void __exit chr_dev_exit(void){
  unregister_chrdev_region(dev, 1);
  cdev_del(my_cdev);
  printk(KERN_INFO"device removed.. \n");
}


module_init(chr_dev_init);
module_exit(chr_dev_exit);
