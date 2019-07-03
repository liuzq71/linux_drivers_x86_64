/*
usage: Compiling this file by make 
and using the following command to insert the mode which the make generated just now

command: sudo insmod filename.ko irq=1 devname=myirq

This interrupt shared the one irq with keyboard
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#include <linux/kthread.h>
#include <linux/semaphore.h>

//#include <linux/mutex.h>
//#include <linux/kdev_t.h>
//#include <linux/cdev.h>a
//#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/kernel.h>

#if defined(CONFIG_ARCH_MESON64_ODROIDC2)
#include <linux/amlogic/iomap.h>
#endif

#define irq_no IRQ_EINT2
//typedef unsigned int __u32;

struct gpioctrl_iocreg  {
        __u32   reg_offset;
        __u32   reg_data;
        __u32   bit_mask;
        __u32   bit_data;
};

#define GPIOCTRL_IOCGREG  _IOR('g',1,struct gpioctrl_iocreg)
#define GPIOCTRL_IOCWREG  _IOW('g',2,struct gpioctrl_iocreg)

static int irq_no=125;   //GPIOX_19 gpio interrupt 


static int irq;
static char* devname;
static struct tasklet_struct mytasklet;

//pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//初始化互斥锁  
//static DEFINE_MUTEX(sem);
//DECLARE_MUTEX(sem2);
//struct semaphore  sem;         //定义信号量

//首先是定义一个结构体，其实这个结构体存放的是一个列表，这个
//列表保存的是一系列设备文件，SIGIO信号就发送到这些设备上
static struct fasync_struct *fasync_queue;

module_param(irq,int,0644);
module_param(devname,charp,0644);

struct myirq
{
	int devid;
};
struct myirq mydev={1119};


//fasync方法的实现
static int my_fasync(int fd, struct file * filp, int on)
{
    int retval;  
    //将该设备登记到fasync_queue队列中去
    retval=fasync_helper(fd,filp,on,&fasync_queue);  
    if(retval<0)
    {
        return retval;
    }
    return 0;
}

static void mytasklet_handler(unsigned long data)
{
	printk("tasklet is wroking..\n");
//down_interruptible(&sem);    //获取信号量
//up(&sem);                              //释放信号量
}

static irqreturn_t myirq_handler(int irq,void* dev)
{
	struct myirq mydev;
	static int count=0;
	mydev=*(struct myirq*)dev;
	printk("key:%d..\n",count+1);
	printk("devid:%d ISR is working..\n",mydev.devid);
	printk("Bottom half will be working..\n");
	tasklet_init(&mytasklet,mytasklet_handler,0);
	tasklet_schedule(&mytasklet);
	printk("ISR is leaving..\n");
	count++;
	return IRQ_HANDLED;
}


static int misc_gpioctrl_open(struct inode *inode, struct file *file)
{
	return 0;
}


static long misc_gpioctrl_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
//////////
}


//在驱动的release方法中我们再调用my_fasync方法

static int misc_gpioctrl_release(struct inode *inode, struct file *filp)
{
   //..processing..
my_fasync(-1, filp, 0);
   //..processing..
}

static struct file_operations chardev_fops = {  
        .owner	= THIS_MODULE,
        .open = misc_gpioctrl_open,  
        .read = NULL,//chardev_read,  
        .write = NULL,//chardev_write, 
.unlocked_ioctl	= misc_gpioctrl_ioctl, 
        .fasync = NULL,//chardev_fasync,  
        .release = misc_gpioctrl_release,  
};  



static int odroid_gpioctrl_suspend(struct platform_device *dev,
					pm_message_t state)
{
	return 0;
}

/*---------------------------------------------------------------------------*/
static int odroid_gpioctrl_resume(struct platform_device *dev)
{
	return 0;
}

/*---------------------------------------------------------------------------*/
static  int odroid_gpioctrl_remove(struct platform_device *pdev)
{
//	if (pmisc)
//		misc_deregister(pmisc);

	return 0;
}
/*---------------------------------------------------------------------------*/


static int odroid_gpioctrl_probe(struct platform_device *pdev)
{
/////////
/* if(request_irq(irq,myirq_handler,IRQF_SHARED,devname,&mydev)!=0)
        {
                printk("%s request IRQ:%d failed..\n",devname,irq);
                return -1;
        }
printk("%s rquest IRQ:%d success..\n",devname,irq);*/

}

static struct platform_driver odroid_gpioctrl_driver = {
	.driver = {
		.name = "odroid-gpioctrl",
		.owner = THIS_MODULE,
#if defined(CONFIG_OF)
		.of_match_table = NULL,//of_match_ptr(odroid_gpioctrl_dt),
#endif
	},
	.probe = odroid_gpioctrl_probe,
	.remove = odroid_gpioctrl_remove,
	.suspend = odroid_gpioctrl_suspend,
	.resume = odroid_gpioctrl_resume,
};

static int __init myirq_init(void)
{
int ret=0;    
// struct semaphore  sem;         //定义信号量
	printk("Module is working..\n");
	if(request_irq(irq,myirq_handler,IRQF_SHARED,devname,&mydev)!=0)
	{
		printk("%s request IRQ:%d failed..\n",devname,irq);
		return -1;
	}
	printk("%s rquest IRQ:%d success..\n",devname,irq);
ret= platform_driver_register(&odroid_gpioctrl_driver);	

if (ret){
        printk(KERN_ERR "failed to register\n");
    }
printk("ret=%d\n",ret);
return ret;
     //struct semaphore  sem;         //定义信号量

//     sema_init(&sem,1);                //初始化信号量

//     down_interruptible(&sem);    //获取信号量
//////////////                                         //临界区代码
//////////////
//     up(&sem);                              //释放信号量
//     return 0;
}

static void __exit myirq_exit(void)
{
	printk("Module is leaving..\n");
	free_irq(irq,&mydev);
	printk("%s request IRQ:%d success..\n",devname,irq);
platform_driver_unregister(&odroid_gpioctrl_driver);
}



module_init(myirq_init);
module_exit(myirq_exit);
MODULE_LICENSE("GPL");


