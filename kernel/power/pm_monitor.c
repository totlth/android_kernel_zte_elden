#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/rtc.h>


#define ZTE_GET_AMSS_SLEEP_TIME
//ZTE_POWER_ZENGHUIPENG_20110212 add
#ifdef ZTE_GET_AMSS_SLEEP_TIME
#include <mach/boot_shared_imem_cookie.h>
#include <mach/msm_iomap.h>

static struct boot_shared_imem_cookie_type *zte_imem_ptr = 
    (struct boot_shared_imem_cookie_type *)MSM_IMEM_BASE;

static ssize_t pm_monitor_amss_sleep_time_show(struct device *devp, struct device_attribute *attr, char *buf)
{
	char * echo = buf;	
	int msleeptimeinsecs=zte_imem_ptr->modemsleeptime;
	if(msleeptimeinsecs==-1)
		msleeptimeinsecs=0;
	
	echo += sprintf(echo, "%d", (int)(msleeptimeinsecs/100));
	
	return echo-buf;
}
static DEVICE_ATTR(amss_sleep_time, S_IRUGO, pm_monitor_amss_sleep_time_show, NULL);
#endif



/*
 * pm_monitor_init
 */
#define PM_MINOR_DEV 150

 static struct miscdevice pm_monitor_device = {
	 .minor 	 = MISC_DYNAMIC_MINOR,
	 .name		 = "pm_monitor",
	 .fops		 = NULL,
 };


 static struct pm_monitor *dev = NULL;
static int __init pm_monitor_init(void)
{
	int ret = -1;

	printk("%s enter\n",__FUNCTION__);

	ret = misc_register(&pm_monitor_device);
	if (ret)
	    goto err;
      

	ret = device_create_file(pm_monitor_device.this_device,&dev_attr_amss_sleep_time);

	if (ret)
		goto out_unregister;

	return 0;

out_unregister: 
	misc_deregister(&pm_monitor_device);
 err:
       

        return ret;
}


/*
 *pm_monitor_exit
 */
static void __exit pm_monitor_exit(void)
{
	printk("%s enter\n",__FUNCTION__);
	

	misc_deregister(&pm_monitor_device);
	kfree(dev);
	
	printk("%s exit\n",__FUNCTION__);
}

MODULE_LICENSE("GPL");

module_init(pm_monitor_init);
module_exit(pm_monitor_exit);
