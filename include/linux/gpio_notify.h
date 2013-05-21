#ifndef __GPIO_NOTIFY_H_
#define __GPIO_NOTIFY_H_
struct gpio_notify_entry
{
	int (*config)(struct gpio_notify_entry *entry, int config);
	int gpio;
	char *name;
	char *l;
	char *h;
	int delay;
};

struct gpio_notify_platform_data
{
	struct gpio_notify_entry *array;
	int size;
};
#endif
