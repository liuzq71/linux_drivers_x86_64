#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/device.h>

#include "memdev.h"

static int mem_major = MEMDEV_MAJOR;
module_param(mem_major, int, S_IRUGO);

struct mem_dev *mem_devp; /*设备结构体指针*/

struct cdev cdev;
struct class *class = NULL;
struct device *device = NULL;

/*文件打开函数*/
static int mem_open(struct inode *inode, struct file *filp)
{
   struct mem_dev *dev;

   /*获取次设备号:区分同类型设备*/
   int num = MINOR(inode->i_rdev);

   if(num >= MEMDEV_NR_DEVS)
      return -ENODEV;
   dev = &mem_devp[num];
   /*将设备描述结构指针赋值给文件私有数据指针*/
   filp->private_data = dev;

   return 0;
}

/*文件释放函数*/
static int mem_release(struct inode *inode, struct file *filp)
{
   return 0;
}

/*读函数*/
static ssize_t
mem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
   unsigned long p = *ppos;
   unsigned int count = size;
   int ret = 0;
   struct mem_dev *dev = filp->private_data; /*获得设备结构体指针*/

   /*判断读位置是否有效*/
   if (p >= MEMDEV_SIZE)
      return 0;
   if (count > MEMDEV_SIZE - p)
      count = MEMDEV_SIZE - p;

   /*读数据到用户空间*/
  if (copy_to_user(buf, (void*)(dev->data + p), count)){
  ret = -EFAULT;
  }
  else{
     *ppos += count;
     ret = count;
     printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
  }
  return ret;
}

/*写函数*/
static ssize_t
mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
   unsigned long p = *ppos;
   unsigned int count = size;
   int ret = 0;
   struct mem_dev *dev = filp->private_data; /*获得设备结构体指针*/

   /*分析和获取有效的写长度*/
   if (p >= MEMDEV_SIZE)
     return 0;
   if (count > MEMDEV_SIZE - p)
     count = MEMDEV_SIZE - p;

   /*从用户空间写入数据*/
   if (copy_from_user(dev->data + p, buf, count))
   ret = -EFAULT;
   else{
     *ppos += count;
     ret = count;

     printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
   }

   return ret;
}


/* seek文件定位函数 */
static loff_t mem_llseek(struct file *filp, loff_t offset, int whence)
{
  loff_t newpos;

  switch(whence) {
    case 0: /* 文件头 */
       newpos = offset;
       break;
    case 1: /* 文件当前位置 */
       newpos = filp->f_pos + offset;
       break;
    case 2: /* 文件末尾 */
       newpos = MEMDEV_SIZE -1 + offset;
       break;
    default: /* can't happen */
       return -EINVAL;
  }
  if ((newpos<0) || (newpos>MEMDEV_SIZE))
    return -EINVAL;

  filp->f_pos = newpos;
  return newpos;
}

/*文件操作结构体*/
static const struct file_operations mem_fops = {
  .owner = THIS_MODULE,
  .llseek = mem_llseek,
  .read = mem_read,
  .write = mem_write,
  .open = mem_open,
  .release = mem_release,
};

/*设备驱动模块加载函数*/
static int memdev_init(void)
{
  int result;
  int i;
  dev_t devno = MKDEV(mem_major, 0);// 合并主设备号/次设备号为设备号
  /* 静态注册设备号*/
  if (mem_major)
    result = register_chrdev_region(devno, 2, "memdev");
  else { /* 动态分配设备号 */
    result = alloc_chrdev_region(&devno, 0, 2, "memdev");
    mem_major = MAJOR(devno);// 获取主设备号
  }

  if (result < 0)
    return result;

  /*初始化cdev结构*/
  cdev_init(&cdev, &mem_fops);
  cdev.owner = THIS_MODULE;
  //cdev.ops = &mem_fops;

  /* 注册字符设备 */
  cdev_add(&cdev, MKDEV(mem_major, 0), MEMDEV_NR_DEVS);

/* 为设备描述结构分配内存*/
  mem_devp = kmalloc(MEMDEV_NR_DEVS * sizeof(struct mem_dev), GFP_KERNEL);
  if(!mem_devp){/*申请失败*/
    result = -ENOMEM;
    goto fail_malloc;
  }
  memset(mem_devp, 0, sizeof(struct mem_dev));// 清空设备描述结构体内存

  /*为设备分配内存*/
  for(i=0;i<MEMDEV_NR_DEVS;i++){
    mem_devp[i].size = MEMDEV_SIZE;
    mem_devp[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
    if (NULL == mem_devp[i].data){/*申请失败*/
      result =-ENOMEM;
      goto fail_malloc;
    }
    memset(mem_devp[i].data, 0, MEMDEV_SIZE);
  }
  /*自动创建设备文件*/
  class = class_create(THIS_MODULE,"test_char"); /*在sys下创建类目录/sys/class/test_char*/
  device_create(class, NULL, MKDEV(mem_major,0), NULL, "memdev0");

  return 0;

fail_malloc:
  cdev_del(&cdev);
  unregister_chrdev_region(devno, 1);
  return result;
}

/*模块卸载函数*/
static void memdev_exit(void)
{
  device_destroy(class, MKDEV(mem_major,0));// 销毁设备
  class_unregister(class);// 注销类
  cdev_del(&cdev);/*注销设备*/
  kfree(mem_devp);/*释放设备结构体内存*/
  unregister_chrdev_region(MKDEV(mem_major, 0), 2); /*释放设备号*/
}

module_init(memdev_init);
module_exit(memdev_exit);

MODULE_LICENSE("GPL");
