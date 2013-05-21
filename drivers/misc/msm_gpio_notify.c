#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>
#include <linux/gpio_notify.h>

#define PM8921_GPIO_BASE		NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8921_GPIO_BASE)

struct notify_entry {
	unsigned int          gpio;
	//int          notify;
	char         *notify_strings[2];
	char         *name;
	char         *l;
	char         *h;
	struct list_head     entry;
	int (*callback)(void *);
	void *data;
	unsigned long delay;
	struct delayed_work	detect;
	struct device *dev;
};
struct gpio_notify_dev {
	struct device *dev;
	struct list_head     gpio;
	spinlock_t	     lock;
	int  index;
};

struct gpio_notify_dev *gpio_notify_dev = NULL;
static struct class *android_class;

static void gpio_notify(struct work_struct *work);

static void free_notify_entry(struct notify_entry* n)
{
	if (NULL == n) {
		return ;
	}
	if (n->name) {
		kfree(n->name);
	}
	if (n->l) {
		kfree(n->l);
	}
	if (n->h) {
		kfree(n->h);
	}
	kfree(n);
	return ;
}


struct notify_entry* alloc_notify_entry(unsigned int gpio, const char *name, const char *h, const char *l)
{
	struct notify_entry *n = NULL;
	
	n = kzalloc(sizeof(struct notify_entry), GFP_KERNEL);
	if (NULL == n) {
		return NULL;
	}
	n->name = kzalloc(strlen(name) + 1, GFP_KERNEL);
	if (NULL == n->name) {
		free_notify_entry(n);
		return NULL;
	}
	strncpy(n->name, name, strlen(name));

	if (NULL != h && NULL != l) {
		n->h = kzalloc(strlen(h) + 1, GFP_KERNEL);
		n->l = kzalloc(strlen(l) + 1, GFP_KERNEL);
		if (!(n->h && n->l)) {
			free_notify_entry(n);
			return NULL;
		}
		strncpy(n->l, l, strlen(l));
		strncpy(n->h, h, strlen(h));
	}
	n->gpio = gpio;
	INIT_LIST_HEAD(&n->entry);
	INIT_DELAYED_WORK(&n->detect, gpio_notify);
	return n;
}
static void gpio_detect_change(struct notify_entry *n, unsigned long delay)
{
	if (NULL == n) {
		printk(KERN_ERR"(%s:%d)invalid parameter\n", __FUNCTION__, __LINE__);
		return ;
	}
	schedule_delayed_work(&n->detect, delay);
}

static irqreturn_t gpio_notify_interrupt(int irq, void *id)
{
	struct notify_entry *n = id;
	//n->notify = 1;
	gpio_detect_change(n, n->delay);
	return IRQ_HANDLED;
}

static int init_gpio(struct notify_entry *n)
{
	int err = 0;
	if (NULL == n) {
		return -1;
	}
	err = gpio_request(n->gpio, n->name);
	if (err < 0) {
		printk(KERN_ERR"(%s:%d)Unable to request gpio %d %s\n",
		       __FUNCTION__, __LINE__, n->gpio, n->name);
		return err;
	}
	gpio_direction_input(n->gpio);
	return 0;
}

#include <linux/mfd/pm8xxx/gpio.h>
static void pm_gpio_config(unsigned int gpio)
{
	struct pm_gpio enable = {
		.direction      = PM_GPIO_DIR_IN,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel        = PM8058_GPIO_VIN_S3,
		.function       = PM_GPIO_FUNC_NORMAL,
		.inv_int_pol    = 0,
	};
	pm8xxx_gpio_config(gpio, &enable);
}

static int init_pmic_gpio(struct notify_entry *n)
{
	int err = 0;
	if (NULL == n) {
		return -1;
	}
	err = gpio_request(n->gpio, n->name);
	if (err < 0) {
		printk(KERN_ERR"(%s:%d)Unable to request gpio %d %s\n",
		       __FUNCTION__, __LINE__, n->gpio, n->name);
		return err;
	}
	pm_gpio_config(n->gpio);
	gpio_direction_input(n->gpio);
	return 0;
}

static void deinit_gpio(struct notify_entry *n)
{
	if (NULL == n) {
		return ;
	}
	gpio_free(n->gpio);

}

static void deinit_pmic_gpio(struct notify_entry *n)
{
	if (NULL == n) {
		return ;
	}
	gpio_free(n->gpio);

}


static ssize_t gpio_value_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct notify_entry *n = dev_get_drvdata(dev);
	if (n->h && n->l) {
		return snprintf(buf, PAGE_SIZE, "%s\n",
				gpio_get_value(n->gpio)?n->h:n->l);
	} else {
		return snprintf(buf, PAGE_SIZE, "%d\n",
				gpio_get_value(n->gpio));
	}
	
}

static ssize_t gpio_value_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

static DEVICE_ATTR(value, S_IRUGO | S_IWUSR,
				        gpio_value_show,
					gpio_value_store);

static struct device_attribute *gpio_attributes[] = {
	&dev_attr_value,
	NULL
};

static int install_notify(struct notify_entry *n)
{
	int err = 0;
	unsigned long			flags;
	struct gpio_notify_dev *dev =  gpio_notify_dev;
	struct device_attribute **attrs = gpio_attributes;
	struct device_attribute *attr;
	int index = 0;

	if (NULL == n) {
		printk(KERN_ERR"(%s:%d)invalid parameter\n",
		       __FUNCTION__, __LINE__);
		return -1;
	}

	err = request_irq(gpio_to_irq(n->gpio),
			  gpio_notify_interrupt,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			  n->name, n);
	if (err < 0) {
		printk(KERN_ERR"(%s:%d)Unable to acquire interrupt gpio %d %s\n",
		       __FUNCTION__, __LINE__, n->gpio, n->name);
		return err;
	}
	spin_lock_irqsave(&dev->lock, flags);
	index = dev->index;
	dev->index++;
	spin_unlock_irqrestore(&dev->lock, flags);
	n->dev = device_create(android_class, dev->dev,
			       MKDEV(0, index), n, n->name);
	if (NULL == n->dev) {
		printk(KERN_ERR"(%s:%d)Unable to create device gpio %d %s\n",
		       __FUNCTION__, __LINE__, n->gpio, n->name);
		free_irq(gpio_to_irq(n->gpio), n);
		return -1;
	}
	err = 0;
	if (attrs) {
		while ((attr = *attrs++) && !err)
			err = device_create_file(n->dev, attr);
	}
	if (err) {
		printk(KERN_ERR"(%s:%d)Unable to create device gpio %d %s\n",
		       __FUNCTION__, __LINE__, n->gpio, n->name);
		free_irq(gpio_to_irq(n->gpio), n);
		device_destroy(android_class, n->dev->devt);
		return -1;
	}

	spin_lock_irqsave(&dev->lock, flags);
	list_add_tail(&n->entry, &gpio_notify_dev->gpio);
	spin_unlock_irqrestore(&dev->lock, flags);
	gpio_detect_change(n, HZ);
	return 0;
}

static void uninstall_notify(struct notify_entry *n)
{
	unsigned long			flags;
	struct gpio_notify_dev *dev =  gpio_notify_dev;
	struct device_attribute **attrs = gpio_attributes;
	struct device_attribute *attr;

	if (NULL == n) {
		printk(KERN_ERR"(%s:%d) invalid parameter\n",
		       __FUNCTION__, __LINE__);
		return ;
	}

	spin_lock_irqsave(&dev->lock, flags);
	list_del(&n->entry);
	spin_unlock_irqrestore(&dev->lock, flags);
	free_irq(gpio_to_irq(n->gpio), n);
	cancel_delayed_work_sync(&n->detect);
	while ((attr = *attrs++))
		device_remove_file(n->dev, attr);
	device_destroy(android_class, n->dev->devt);
}

static ssize_t
gpio_notify_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct gpio_notify_dev *dev = dev_get_drvdata(pdev);
	char *buff = buf;
	struct notify_entry *n = NULL;
	list_for_each_entry(n, &dev->gpio, entry) {
		buff += snprintf(buff, PAGE_SIZE, "%s %u %s %s\n", n->name, n->gpio, n->h, n->l);
	}
	return buff - buf;
}

static ssize_t
new_gpio_notify_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct gpio_notify_dev *dev = dev_get_drvdata(pdev);

	unsigned gpio = 0;
	char name[36] = {0};
	char h[36] = {0};
	char l[36] = {0};
	struct notify_entry *n = NULL;
	int  found = 0;

	int err;
	err = sscanf(buff, "%u %s %s %s", &gpio, name, h, l);
	printk(KERN_ERR"(%s:%d) gpio:%u name:%s h:%s l:%s err=%d\n",
	       __FUNCTION__, __LINE__,
	       gpio, name, h, l, err);
	list_for_each_entry(n, &dev->gpio, entry) {
		if (n->gpio == gpio) {
			found = 1;
			break;
		}
	}

	if (!found) {
		n = NULL;
		n = alloc_notify_entry(gpio, name, h, l);
		if (n == NULL) {
			printk(KERN_ERR"(%s:%d) alloc notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       gpio, name, h, l);
			return size;
		}
		if (init_gpio(n)) {
			printk(KERN_ERR"(%s:%d) install notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       gpio, name, h, l);
			free_notify_entry(n);
			return -1;
		}
		if (install_notify(n)) {
			printk(KERN_ERR"(%s:%d) install notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       gpio, name, h, l);
			deinit_gpio(n);
			free_notify_entry(n);
		}
	}

	return size;
}

static ssize_t
remove_gpio_notify_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct gpio_notify_dev *dev = dev_get_drvdata(pdev);

	unsigned gpio = 0;
	char name[36] = {0};
	char h[36] = {0};
	char l[36] = {0};
	struct notify_entry *n = NULL;
	int  found = 0;

	int err;
	err = sscanf(buff, "%u %s %s %s", &gpio, name, h, l);
	list_for_each_entry(n, &dev->gpio, entry) {
		if (n->gpio == gpio) {
			found = 1;
			break;
		}
	}
	if (found) {
		uninstall_notify(n);
		deinit_gpio(n);
		free_notify_entry(n);
	}

	return size;
}

static ssize_t
pmic_new_gpio_notify_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct gpio_notify_dev *dev = dev_get_drvdata(pdev);

	unsigned gpio = 0;
	char name[36] = {0};
	char h[36] = {0};
	char l[36] = {0};
	struct notify_entry *n = NULL;
	int  found = 0;
	int err;
	err = sscanf(buff, "%u %s %s %s", &gpio, name, h, l);

	gpio = PM8921_GPIO_PM_TO_SYS(gpio);
	printk(KERN_ERR"(%s:%d) gpio:%u name:%s h:%s l:%s err=%d\n",
	       __FUNCTION__, __LINE__,
	       gpio, name, h, l, err);
	list_for_each_entry(n, &dev->gpio, entry) {
		if (n->gpio == gpio) {
			found = 1;
			break;
		}
	}

	if (!found) {
		n = NULL;
		n = alloc_notify_entry(gpio, name, h, l);
		if (n == NULL) {
			printk(KERN_ERR"(%s:%d) alloc notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       gpio, name, h, l);
			return size;
		}

		if (init_pmic_gpio(n)) {
			printk(KERN_ERR"(%s:%d) install notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       gpio, name, h, l);
			free_notify_entry(n);
			return size;
		}


		if (install_notify(n)) {
			printk(KERN_ERR"(%s:%d) install notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       gpio, name, h, l);
			deinit_pmic_gpio(n);
			free_notify_entry(n);
		}

	}

	return size;
}

static ssize_t
pmic_remove_gpio_notify_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct gpio_notify_dev *dev = dev_get_drvdata(pdev);

	unsigned gpio = 0;
	char name[36] = {0};
	char h[36] = {0};
	char l[36] = {0};
	struct notify_entry *n = NULL;
	int  found = 0;

	int err;
	err = sscanf(buff, "%u %s %s %s", &gpio, name, h, l);
	gpio = PM8921_GPIO_PM_TO_SYS(gpio);
	list_for_each_entry(n, &dev->gpio, entry) {
		if (n->gpio == gpio) {
			found = 1;
			break;
		}
	}
	if (found) {
		uninstall_notify(n);
		deinit_pmic_gpio(n);
		free_notify_entry(n);
	}

	return size;
}

static DEVICE_ATTR(new, S_IRUGO | S_IWUSR, gpio_notify_show, new_gpio_notify_store);
static DEVICE_ATTR(remove, S_IRUGO | S_IWUSR, gpio_notify_show, remove_gpio_notify_store);
static DEVICE_ATTR(pmic_new, S_IRUGO | S_IWUSR, gpio_notify_show, pmic_new_gpio_notify_store);
static DEVICE_ATTR(pmic_remove, S_IRUGO | S_IWUSR, gpio_notify_show, pmic_remove_gpio_notify_store);
static struct device_attribute *gpio_notify_attributes[] = {
	&dev_attr_new,
	&dev_attr_remove,
	&dev_attr_pmic_new,
	&dev_attr_pmic_remove,

	NULL,
};

static int  msm_gpio_notify_probe(struct platform_device *pdev)
{
	struct gpio_notify_platform_data  *plat;
	int i = 0;
	struct notify_entry *n = NULL;
	plat = pdev->dev.platform_data;
	for (i = 0; i < plat->size; i++) {
		struct gpio_notify_entry *e = &plat->array[i];
		if (!(e && e->h && e->l && e->name)) {
			printk(KERN_ERR"(%s:%d) parameter invalid\n",
			       __FUNCTION__, __LINE__);
			goto err;
		}
		n = alloc_notify_entry(e->gpio, e->name, e->h, e->l);
		if (n == NULL) {
			printk(KERN_ERR"(%s:%d) alloc notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       e->gpio, e->name, e->h, e->l);
			goto err;
		}
		n->delay = e->delay;
		if (e->config(e, 1)) {
			free_notify_entry(n);
			goto err;
		}

		if (install_notify(n)) {
			printk(KERN_ERR"(%s:%d) install notify fail gpio:%u name:%s h:%s l:%s\n",
			       __FUNCTION__, __LINE__,
			       e->gpio, e->name, e->h, e->l);
			e->config(e, 0);
			free_notify_entry(n);
			goto err;
		}
	}
	

	return 0;
err:
	{
		struct gpio_notify_dev *dev = gpio_notify_dev;
		n = NULL;
		list_for_each_entry(n, &dev->gpio, entry) {
			uninstall_notify(n);
			free_notify_entry(n);
		}
	}

	return -1;
}

static struct platform_driver msm_gpio_notify_driver = {
	.probe = msm_gpio_notify_probe,
	.driver = {
		.name = "msm_gpio_notify",
	},
};



char buf0[128] = {0};
char buf1[128] = {0};
char buf2[128] = {0};

static void gpio_notify(struct work_struct *work)
{
	struct notify_entry *n = 
		container_of(work, struct notify_entry, detect.work);
	struct gpio_notify_dev *dev = gpio_notify_dev;
	if (NULL != n->callback) {
		n->callback(n->data);
	}
	if (n->l && n->h) {
		char *status[4]    = { NULL, NULL, NULL, NULL };
		memset(buf0, sizeof(buf0), 0);
		memset(buf1, sizeof(buf1), 0);
		snprintf(buf0, sizeof(buf0), "gpio=%u", n->gpio);
		snprintf(buf1, sizeof(buf1), "status=%s", gpio_get_value(n->gpio)?n->h:n->l);
		snprintf(buf2, sizeof(buf2), "name=%s", n->name);
		status[0] = buf0;
		status[1] = buf1;
		status[2] = buf2;
		kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE,
				   status);
	}

}


static int gpio_create_device(struct gpio_notify_dev *dev)
{
	struct device_attribute **attrs = gpio_notify_attributes;
	struct device_attribute *attr = NULL;
	int err;

	dev->dev = device_create(android_class, NULL,
					MKDEV(0, 0), NULL, "notify");
	if (IS_ERR(dev->dev))
		return PTR_ERR(dev->dev);

	dev_set_drvdata(dev->dev, dev);

	while ((attr = *attrs++)) {
		err = device_create_file(dev->dev, attr);
		if (err) {
			device_destroy(android_class, dev->dev->devt);
			return err;
		}
	}
	return 0;
}

static void gpio_destroy_device(struct gpio_notify_dev *dev)
{
	struct device_attribute **attrs = gpio_notify_attributes;
	struct device_attribute *attr;

	while ((attr = *attrs++))
		device_remove_file(dev->dev, attr);
	device_destroy(android_class, dev->dev->devt);
}


static int __init msm_gpio_notify_init(void)
{
	struct gpio_notify_dev *dev;
	int ret = 0;
	android_class = class_create(THIS_MODULE, "gpio_notify");
	if (IS_ERR(android_class))
		return PTR_ERR(android_class);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		pr_err("%s(): Failed to alloc memory for gpio_notify\n",
				__func__);
		class_destroy(android_class);
		return -ENOMEM;
	}
	spin_lock_init(&dev->lock);
	INIT_LIST_HEAD(&dev->gpio);
	ret = gpio_create_device(dev);
	if (ret) {
		pr_err("%s(): gpio_create_device failed\n", __func__);
		goto err_dev;
	}
	dev->index = 1;
	gpio_notify_dev = dev;
	ret = platform_driver_register(&msm_gpio_notify_driver);
	if (ret) {
		pr_err("%s(): Failed to register android"
				 "platform driver\n", __func__);
		goto err_probe;
	}
	return ret;
err_probe:
	gpio_destroy_device(dev);
err_dev:
	kfree(dev);
	class_destroy(android_class);
	return ret;
}

static void __exit msm_gpio_notify_exit(void)
{
	struct notify_entry *n = NULL;
	struct gpio_notify_dev *dev = gpio_notify_dev;
	list_for_each_entry(n, &dev->gpio, entry) {
		uninstall_notify(n);
		free_notify_entry(n);
	}
	platform_driver_unregister(&msm_gpio_notify_driver);
}

module_init(msm_gpio_notify_init);
module_exit(msm_gpio_notify_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MSM gpio notify");
