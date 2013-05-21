/*
 * drivers/staging/zte_ftm/ftm.c
 *
 * Copyright (C) 2012-2013 ZTE, Corp.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  Date         Author           Comment
 *  -----------  --------------   -------------------------------------
 *  2012-08-03	 Jia   	          The kernel module for FTM,
 *                                created by ZTE_BOO_JIA_20120803 jia.jia
 *  -------------------------------------------------------------------
 *
*/

#include <linux/module.h>
#include <linux/sysdev.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/scm.h>
#include <mach/msm_iomap.h>
#include <mach/msm_iomap.h>
#include <mach/boot_shared_imem_cookie.h>

/*
 * Macro Definition
 */
#define FTM_VERSION "1.0"
#define FTM_CLASS_NAME "zte_ftm"

#define QFPROM_WRITE_ROW_ID				0x03
#define QFPROM_WRITE_MULTIPLE_ROWS_ID	0x04
#define QFPROM_READ_ROW_ID				0x05
#define QFPROM_ROLLBACK_WRITE_ROW_ID	0x06

#define FUSION_OPT_UNDEFINE		0x00
#define FUSION_OPT_WRITE_SECURE_BOOT		0x01
#define FUSION_OPT_READ_SECURE_BOOT	       0x02
#define FUSION_OPT_WRITE_SIMLOCK			0x03
#define FUSION_OPT_READ_SIMLOCK			0x04

#define FUSION_MAGIC_NUM_1      0x46555349
#define FUSION_MAGIC_NUM_2      0x4F4E2121

#define QFPROM_STATUS_FUSE_OK		0x01
#define QFPROM_STATUS_FUSE_FAIL		0x00
#define QFPROM_STATUS_UNKOWN	0xff

/*
 * Type Definition
 */
typedef struct {
	int32_t fusion;
	int32_t simlocking;
} ftm_sysdev_attrs_t;

typedef struct {
    /* Row address */
	uint32_t	raw_row_address;
    /* MSB row data */
	uint32_t	row_data_msb;	
    /* LSB row data */
	uint32_t	row_data_lsb;
} write_row_type;

typedef enum
{
  QFPROM_ADDR_SPACE_RAW = 0,            /**< Raw address space. */
  QFPROM_ADDR_SPACE_CORR,               /**< Corrected address space. */
  QFPROM_ADDR_SPACE_MAX = 0x7FFFFFFF    /**< Last entry in the QFPROM_ADDR_SPACE enumerator.*/
} QFPROM_ADDR_SPACE;

typedef struct {
	uint32_t row_address;
	uint32_t addr_type;
	uint32_t *fuse_data;
	uint32_t *status;
} read_row_cmd;
/*
 * Global Variables Definition
 */
static struct sys_device ftm_sysdev;
static ftm_sysdev_attrs_t ftm_sysdev_attrs;
static spinlock_t ftm_spinlock;

static uint32_t fuse_data_buf[2] = {0};
static uint32_t status_buf = 0;

/*
 * Function declaration
 */

static int32_t ftm_register_sysdev(struct sys_device *sysdev);

/*
 * Function Definition
 */
static ftm_fusion_info * get_fusion_info(void)
{

/*Define a global pointer which points to the boot shared imem cookie structure */
struct boot_shared_imem_cookie_type *boot_shared_imem_cookie_ptr = 
                  (struct boot_shared_imem_cookie_type *)MSM_IMEM_BASE;
/*Define a global pointer which points to the boot shared imem cookie structure */
if(boot_shared_imem_cookie_ptr == NULL)
	return (ftm_fusion_info *)NULL;
else
	return &(boot_shared_imem_cookie_ptr->fusion_info);
}


/*
 * Function Definition
 */
static ssize_t show_fusion(struct sys_device *dev, struct sysdev_attribute *attr, char *buf)
{
	ftm_fusion_info * ftm_fusion_info_ptr = get_fusion_info();
	
	pr_info("%s: e\n", __func__);

	if(ftm_fusion_info_ptr != NULL
		&& ftm_fusion_info_ptr->magic_1 == FUSION_MAGIC_NUM_1
		&& ftm_fusion_info_ptr->magic_2 == FUSION_MAGIC_NUM_2)
	{
		ftm_sysdev_attrs.fusion = ftm_fusion_info_ptr->fusion_status;
		
		ftm_fusion_info_ptr->magic_1 = 0;
		ftm_fusion_info_ptr->magic_2 = 0;
		ftm_fusion_info_ptr->fusion_type = FUSION_OPT_UNDEFINE;
		ftm_fusion_info_ptr->fusion_status = QFPROM_STATUS_UNKOWN;		
	}
	else
	{
		ftm_sysdev_attrs.fusion = QFPROM_STATUS_UNKOWN;
	  pr_info("%s: fusion magic error\n",__func__);		
	}	
	pr_info("%s:fusion_status %x \n",__func__, ftm_sysdev_attrs.fusion);
				
	pr_info("%s: ex\n", __func__);
	
	return snprintf(buf, PAGE_SIZE, "%d\n", ftm_sysdev_attrs.fusion);
}

static ssize_t store_fusion(struct sys_device *dev, struct sysdev_attribute *attr, const char *buf, size_t buf_sz)
{
	ftm_fusion_info * ftm_fusion_info_ptr = get_fusion_info();
	int32_t fusion_type;
	
	pr_info("%s: e\n", __func__);
		
	sscanf(buf, "%d", &fusion_type);
	pr_info("%s:fusion_type %d \n",__func__,fusion_type);
	
	if((fusion_type != FUSION_OPT_WRITE_SECURE_BOOT)
		&&(fusion_type!= FUSION_OPT_WRITE_SIMLOCK))
	{
	  pr_info("%s:fusion_type error\n",__func__);		
	  	
		return  -EFAULT;
	}
	
	if(!ftm_fusion_info_ptr)
	{
	  pr_info("%s:ftm_fusion_info_ptr NULL\n",__func__);	
	  	
		return  -EFAULT;
	}

#if 1	
	__raw_writel(FUSION_MAGIC_NUM_1, MSM_IMEM_BASE+0x18);
	__raw_writel(FUSION_MAGIC_NUM_2, MSM_IMEM_BASE+0x1c);
	__raw_writel(fusion_type, MSM_IMEM_BASE+0x20);
	__raw_writel(QFPROM_STATUS_UNKOWN, MSM_IMEM_BASE+0x24);
		mb();		
#else
	pr_info("%s:ftm_fusion_info_ptr %p \n",__func__,ftm_fusion_info_ptr);	
	ftm_fusion_info_ptr->magic_1 = FUSION_MAGIC_NUM_1;
	ftm_fusion_info_ptr->magic_2 = FUSION_MAGIC_NUM_2;
	ftm_fusion_info_ptr->fusion_type = fusion_type;
	ftm_fusion_info_ptr->fusion_status = QFPROM_STATUS_UNKOWN;
#endif	
	
	pr_info("%s: x\n", __func__);	
	
	return buf_sz;
}

static SYSDEV_ATTR(fusion, S_IRUGO | S_IWUSR, show_fusion, store_fusion);

static ssize_t store_read_flag(struct sys_device *dev, struct sysdev_attribute *attr, const char *buf, size_t buf_sz)
{
	ftm_fusion_info * ftm_fusion_info_ptr = get_fusion_info();
	int32_t fusion_type;
	
	pr_info("%s: e\n", __func__);
		
	sscanf(buf, "%d", &fusion_type);
	pr_info("%s:fusion_type %d \n",__func__,fusion_type);
	
	if((fusion_type != FUSION_OPT_READ_SECURE_BOOT)
		&&(fusion_type!= FUSION_OPT_READ_SIMLOCK))
	{
	  pr_info("%s:fusion_type error\n",__func__);			
	  	
	  return  -EFAULT;
	}
	
	if(!ftm_fusion_info_ptr)
	{
	  pr_info("%s:ftm_fusion_info_ptr NULL\n",__func__);	
	  	
		return  -EFAULT;
	}	
	
#if 1	
	__raw_writel(FUSION_MAGIC_NUM_1, MSM_IMEM_BASE+0x18);
	__raw_writel(FUSION_MAGIC_NUM_2, MSM_IMEM_BASE+0x1c);
	__raw_writel(fusion_type, MSM_IMEM_BASE+0x20);
	__raw_writel(QFPROM_STATUS_UNKOWN, MSM_IMEM_BASE+0x24);
		mb();		
#else
	pr_info("%s:ftm_fusion_info_ptr %p \n",__func__,ftm_fusion_info_ptr);	
	ftm_fusion_info_ptr->magic_1 = FUSION_MAGIC_NUM_1;
	ftm_fusion_info_ptr->magic_2 = FUSION_MAGIC_NUM_2;
	ftm_fusion_info_ptr->fusion_type = fusion_type;
	ftm_fusion_info_ptr->fusion_status = QFPROM_STATUS_UNKOWN;
#endif
	
	pr_info("%s: x\n", __func__);	
	
	return buf_sz;
}

static SYSDEV_ATTR(fusion_type, S_IRUGO | S_IWUSR, NULL, store_read_flag);

static ssize_t show_dump(struct sys_device *dev, struct sysdev_attribute *attr, char *buf)
{
	int ret;
	read_row_cmd cmd_buf;
	size_t max_size;
	size_t used_size;
	size_t n;

	cmd_buf.addr_type = QFPROM_ADDR_SPACE_RAW;
	cmd_buf.fuse_data = (uint32_t *)virt_to_phys(fuse_data_buf);
	cmd_buf.status = (uint32_t *)virt_to_phys(&status_buf);

	max_size = PAGE_SIZE;
	used_size = 0;
	for (cmd_buf.row_address = 0x007000A0; cmd_buf.row_address < 0x007003F8; cmd_buf.row_address += 8) {
		if (cmd_buf.row_address % 0x10 == 0) {
			n = snprintf(buf, max_size, "\n0x%08x:", cmd_buf.row_address);
			buf += n;
			used_size += n;
			max_size -= n;
		}
		fuse_data_buf[0] = 0; fuse_data_buf[1] = 0;
		status_buf = 0;
		ret = scm_call(SCM_SVC_FUSE, QFPROM_READ_ROW_ID, &cmd_buf, sizeof(cmd_buf), NULL, 0);
		if (ret) {
			printk("read fuse return %d %d\n", ret, status_buf);
			n = snprintf(buf, max_size, " ********** **********");
		} else {
			n = snprintf(buf, max_size, " 0x%08x 0x%08x", fuse_data_buf[0], fuse_data_buf[1]);
		}
		buf += n;
		used_size += n;
		max_size -= n;
	}

	used_size += snprintf(buf, max_size, "\n");

	return used_size;
}

static ssize_t store_dump(struct sys_device *dev, struct sysdev_attribute *attr, const char *buf, size_t buf_sz)
{
	return buf_sz;
}

static SYSDEV_ATTR(dump, S_IRUGO | S_IWUSR, show_dump, store_dump);

static struct sysdev_attribute *ftm_attrs[] = {
	&attr_fusion,
	&attr_fusion_type,
	&attr_dump
};

static struct sysdev_class ftm_sysdev_class = {
	.name = FTM_CLASS_NAME
};

/*
 * Sys device register
 *
 * sysdev file:
 *
 * /sys/devices/system/zte_ftm/zte_ftm0/fusion
 * /sys/devices/system/zte_ftm/zte_ftm0/simlocking
 */
static int32_t ftm_register_sysdev(struct sys_device *sysdev)
{
	int32_t ret;
	int32_t i;

	ret = sysdev_class_register(&ftm_sysdev_class);
	if (ret) {
		return ret;
	}

	sysdev->id = 0;
	sysdev->cls = &ftm_sysdev_class;

	ret = sysdev_register(sysdev);
	if (ret) {
		sysdev_class_unregister(&ftm_sysdev_class);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(ftm_attrs); i++) {
		ret = sysdev_create_file(sysdev, ftm_attrs[i]);
		if (ret) {
			goto ftm_fail;
		}
	}

	return 0;

ftm_fail:

	while (--i >= 0) sysdev_remove_file(sysdev, ftm_attrs[i]);

	sysdev_unregister(sysdev);
	sysdev_class_unregister(&ftm_sysdev_class);

	  return ret;
}

static int32_t __init ftm_probe(struct platform_device *pdev)
{
	int ret;
	uint32_t cmd_id;

	for (cmd_id = QFPROM_WRITE_ROW_ID; cmd_id <= QFPROM_ROLLBACK_WRITE_ROW_ID; cmd_id++) {
		ret = scm_is_call_available(SCM_SVC_FUSE, cmd_id);
		printk("ftm: availability of cmd 0x%02x is %d\n", cmd_id, ret);
		if (ret <= 0)
			return ret;
	}

	return 0;
}

static int32_t ftm_remove(struct platform_device *pdev)
{
	pr_info("%s: e\n", __func__);

	pr_info("%s: x\n", __func__);

	return 0;
}

static struct platform_driver ftm_driver = {
	.remove = ftm_remove,
	.driver = {
		.name = "zte_ftm",
		.owner = THIS_MODULE,
	},
};

/*
 * Initializes the module.
 */
static int32_t __init ftm_init(void)
{
	int ret;

	pr_info("%s: e\n", __func__);

	ret = platform_driver_probe(&ftm_driver, ftm_probe);
	if (ret) {
	    return ret;
	}

	/*
	 * Initialize spinlock
	 */
	spin_lock_init(&ftm_spinlock);

	/*
	 * Initialize sys device
	 */
	ftm_register_sysdev(&ftm_sysdev);

	pr_info("%s: x\n", __func__);

	return 0;
}

/*
 * Cleans up the module.
 */
static void __exit ftm_exit(void)
{
	platform_driver_unregister(&ftm_driver);
}

module_init(ftm_init);
module_exit(ftm_exit);

MODULE_DESCRIPTION("ZTE FTM Driver Ver %s" FTM_VERSION);
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
