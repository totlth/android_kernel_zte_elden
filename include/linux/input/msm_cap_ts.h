#ifndef __LINUX_CAP_TS_H__
#define __LINUX_CAP_TS_H__


//#include <linux/input.h>

struct cap_ts_platform_data{
	int has_vkeys;
	int param;
	int points_needed;
	int gpio_irq;
	int (*power)(int on);
};


#endif /* __LINUX_CAP_TS_H__ */
