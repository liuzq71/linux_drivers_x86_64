    #include <linux/module.h> 
    #include <linux/init.h> 
    #include <linux/kernel.h> 
    #include <linux/fs.h> 
    #include <linux/interrupt.h> 
     
    static struct tasklet_struct my_tasklet;    
    //  tasklet处理函数  
    static void tasklet_handler (unsigned long data)  
    {  
           printk(KERN_ALERT "tasklet_handler is running.\n");  
    }    
    static int __init demo_init(void)  
    {  
           //  初始化tasklet  
           tasklet_init(&my_tasklet, tasklet_handler, 0);  
           //  调度tasklet处理程序     
           tasklet_schedule(&my_tasklet);  
           printk(KERN_ALERT "demo_init.\n");  
           return 0;  
    }    
     
    static void __exit demo_exit(void)  
    {  
          //  销毁tasklet  
          tasklet_kill(&my_tasklet);  
          printk(KERN_ALERT "demo_exit.\n");  
    }  
    MODULE_LICENSE("GPL");    
    module_init(demo_init);  
    module_exit(demo_exit);  
