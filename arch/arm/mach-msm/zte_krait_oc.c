/*
 * Overclocking driver for Qualcomm MSM8960 (Krait) devices, such as the
 * HTC One XL (AT&T/Rogers/Asia) and HTC One S. This method is recommended for
 * rooted devices without kernel source or a locked bootloader.
 *
 * Successfully compiled against Code Aurora's ics_chocolate branch MSM kernel,
 * but requires a little tweaking to Module.symvers before it will successfully
 * load.
 *
 * Please encourage HTC and AT&T to allow the AT&T HTC One X's bootloader to be
 * unlocked! Even if it doesn't affect you, and even if it gets S-OFF
 * eventually, this sets a bad precedent for carriers to get in the way of
 * manufacturers' intentions of device freedom.
 * 
 * Copyright (c) 2012 Michael Huang
 * Author: Michael Huang <mike@setcpu.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

 /*
 [read me 20120718:]
 
http://forum.xda-developers.com/showthread.php?t=1665470


One XL/AT&T/Rogers One X/One S 1.8GHz overclock module

Please let me know about the module's compatibility with your phone. It has been reported to work on the latest AT&T version kernel, and I have a Rogers device myself.

Hey guys! I've created a kernel module that overclocks the MSM8960 Qualcomm Krait to over 1.8GHz. It also allows you to specify a custom voltage and frequency instead of 1.8GHz. This method works on ANY MSM8960-based rooted HTC One X, even on locked bootloader devices. This allows AT&T users to overclock, and others to overclock without necessarily needing to unlock the bootloader. This overclock makes the already tremendously fast MSM8960 about 20% faster.

Note that this mod is ONLY for the Qualcomm MSM8960-based Krait HTC One X and HTC One XL. This includes the AT&T HTC One X, Rogers HTC One X, and any phone branded as the HTC One XL. It has also been confirmed to work on the One S (see this thread: http://forum.xda-developers.com/show....php?t=1670904) and MIGHT work on the EVO 4G LTE.

The module is loaded using this command: insmod /data/local/krait_oc.ko pll_l_val=67 vdd_uv=1300000

The pll_l_val parameter determines how high the overclock is. Multiply this number by 27 to get the final clock speed in MHz. For example, 67*27 is 1809000, which is what the module defaults to.

The vdd_uv parameter determines the voltage used at the overclocked speed, in microvolts. The default for 1.5GHz is 1200000, and I was able to get a stable overclock at 1300000 at 1.8GHz. Raise the vdd_uv parameter if the overclock is unstable. The current maximum for this field is 1300000, so don't go higher than this. If your system crashes or is unstable at this frequency/voltage, lower the pll_l_val one by one until you reach stability. You can run rmmod krait_oc and then insmod krait_oc.ko with different parameters without having to reboot.

You'll also need a custom, tweaked thermald.conf. This thermald.conf raises thermal tolerances slightly (I've found that they're a little too strict, even at stock clocks and voltages). I've included this in the package, and instructions for installing it are below.

Video, demonstrating the overclock on a Rogers HTC One X:


Screenshots


Source code is included in the package. If anyone has an HTC One S, this method will work on that, too. Please post below a dump of /system/lib/modules and I should be able to add support for any MSM8960-based HTC device with just that.

Instructions
First, determine which kernel module to use. Do an adb shell cat /proc/version and choose a ko file that matches your version number (these instructions assume you've renamed it to krait_oc.ko). 21/05/2012: If you don't see your kernel version here, try loading the module anyway. If it fails to load, please post a file from /system/lib/modules (any file) here and I will add support.

Install the overclock (only once):
1. Push the kernel module to your device:
Code:
adb push krait_oc.ko /data/local
2. Install the new thermald.conf, making sure to back up the old one, and reboot. The thermald.conf is included in the download. If you want to target a frequency other than 1809000 KHz, you should edit the thermald.conf and replace "1809000" to whatever frequency you want to target.
Code:
adb push thermald.conf /data/local
adb shell
su
mount -o rw,remount /system
cp /system/etc/thermald.conf /system/etc/thermald.conf.bak
rm -r /system/etc/thermald.conf
cp /data/local/thermald.conf /system/etc
reboot
Load the overclock (every time you reboot):
1. Load the kernel module (replace pll_l_val and vdd_uv with your desired voltages and L value as explained above. It defaults to 67 and 1300000 if you don't give it any parameters):
Code:
adb shell
su
insmod /data/local/krait_oc.ko pll_l_val=67 vdd_uv=1300000
2. Bring core 1 temporarily offline so it gets updated with the new max frequency:
Code:
echo 0 > /sys/devices/system/cpu/cpu1/online
3. You'll now have an additional CPU frequency! SetCPU can configure your maximum frequency up to this speed. You can also choose to keep running at 1.5GHz at any time - this method doesn't eliminate any available frequencies. Set the max at 1.8GHz to verify it's stable here.
4. Restart thermald by running "ps". Look for "thermald" in the list, and find thermald's pid (it's usually a number in the low hundreds, higher up in the list). Run "kill [thermald's PID]" in adb shell. The kernel does not currently have kernel-level temperature throttling turned on, so thermald is important for now.

Remove the overclock by restoring your backup of thermald.conf:
Code:
adb shell
su
mount -o rw,remount /system
rm -r /system/etc/thermald.conf
cp /system/etc/thermald.conf.bak /system/etc/thermald.conf
rm -r /system/etc/thermald.conf.bak
reboot
Rebooting clears any kernel modules that are loaded, so you're now clean. You can then delete anything left over in /data/local, but it doesn't matter.

If the module loads but the overclock doesn't seem to have any effect, even after putting max and min at 1.8GHz, your device might use a different SoC bin than the "nominal," and the kernel module is looking at the wrong place. Please reboot your device and post an adb shell dmesg right after the reboot so I can look at it.

Download current pack of modules:
http://www.setcpu.com/files/krait_oc_v2.zip (current)
http://www.setcpu.com/files/krait_oc.zip (old)

Finally, it'd be great if we as a community tried to work harder to encourage HTC to hurry up and 1. Release kernel source on time, all the time and 2. NOT cave into carrier pressure and stick to their written bootloader policy! S-OFF would be nice, too. 

 */

#include <linux/cpufreq.h>
#include <linux/kallsyms.h>

#define DRIVER_AUTHOR "Michael Huang <mike@setcpu.com>"
#define DRIVER_DESCRIPTION "MSM 8960 Overclock Driver"
#define DRIVER_VERSION "1.1"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

/* Speed of the HFPLL in KHz */
#define HFPLL_FREQ_KHZ 27000

/* Name of the acpu_freq_tbl symbols */
#define ACPU_FREQ_TBL_NOM_NAME "acpu_freq_tbl_8960_kraitv2_nom"
#define ACPU_FREQ_TBL_SLOW_NAME "acpu_freq_tbl_8960_kraitv2_slow"
#define ACPU_FREQ_TBL_FAST_NAME "acpu_freq_tbl_8960_kraitv2_fast"

/* Module parameters */
/* PLL L value. Controls overclocked frequency. 27 MHz * pll_l_val = X MHz */
static uint pll_l_val = 0x43;

/* Voltage of the overclocked frequency, in uV. Max supported is 1300000 uV */
static uint vdd_uv = 1300000;

module_param(pll_l_val, uint, 0444);
module_param(vdd_uv, uint, 0444);
MODULE_PARM_DESC(pll_l_val, "Frequency multiplier for overclocked frequency");
MODULE_PARM_DESC(vdd_uv, "Core voltage in uV for overclocked frequency");

/* New frequency table */
static struct cpufreq_frequency_table freq_table[] = {
	{ 0, 384000 },
	{ 1, 486000 },
	{ 2, 594000 },
	{ 3, 702000 },
	{ 4, 810000 },
	{ 5, 918000 },
	{ 6, 1026000 },
	{ 7, 1134000 },
	{ 8, 1242000 },
	{ 9, 1350000 },
	{ 10, 1458000 },
	{ 11, 1512000 },
	/* We replace this row with our desired frequency later */
	{ 12, 1809000 },
	{ 13, CPUFREQ_TABLE_END },
};

struct core_speed {
	unsigned int		khz;
	int			src;
	unsigned int		pri_src_sel;
	unsigned int		sec_src_sel;
	unsigned int		pll_l_val;
};

struct acpu_level {
	unsigned int		use_for_scaling;
	struct core_speed	speed;
	struct l2_level		*l2_level;
	unsigned int		vdd_core;
};

struct cpufreq_frequency_table *orig_table;

/* Use a function pointer for cpufreq_cpu_get because the symbol version
 * differs in the HTC kernel and the Code Aurora kernel, so the kernel won't
 * let us call it normally.
 *
 * Call the function normally when the kernel source is released.
 */
typedef struct cpufreq_policy *(*cpufreq_cpu_get_type)(int); 
struct cpufreq_policy *(*cpufreq_cpu_get_new)(int);

/* Updates a row in a struct acpu_level with symbol name symbol_name */
static void __init acpu_freq_row_update
	(char *symbol_name, uint index, uint l_val, uint vdd)
{
	struct acpu_level *acpu_freq_tbl;
	ulong acpu_freq_tbl_addr;

	acpu_freq_tbl_addr = kallsyms_lookup_name(symbol_name);

	if(acpu_freq_tbl_addr == 0) {
		printk(KERN_WARNING "krait_oc: symbol not found\n");
		printk(KERN_WARNING "krait_oc: skipping this table\n");
		return;
	}

	acpu_freq_tbl = (struct acpu_level*) acpu_freq_tbl_addr;
	acpu_freq_tbl[index].speed.khz = l_val*HFPLL_FREQ_KHZ;
	acpu_freq_tbl[index].speed.pll_l_val = l_val;
	acpu_freq_tbl[index].vdd_core = vdd;
}

static int __init overclock_init(void)
{
	struct cpufreq_policy *policy;
	ulong cpufreq_cpu_get_addr;
	uint cpu;

	printk(KERN_INFO "krait_oc: %s version %s\n", DRIVER_DESCRIPTION,
		DRIVER_VERSION);
	printk(KERN_INFO "krait_oc: by %s\n", DRIVER_AUTHOR);
	printk(KERN_INFO "krait_oc: overclocking to %u at %u uV\n",
		pll_l_val*HFPLL_FREQ_KHZ, vdd_uv);

	printk(KERN_INFO "krait_oc: updating cpufreq policy\n");

	cpufreq_cpu_get_addr = kallsyms_lookup_name("cpufreq_cpu_get");

	if(cpufreq_cpu_get_addr == 0) {
		printk(KERN_WARNING "krait_oc: symbol not found\n");
		printk(KERN_WARNING "krait_oc: not attempting overclock\n");
		return 0;
	}

	cpufreq_cpu_get_new = (cpufreq_cpu_get_type) cpufreq_cpu_get_addr;

	policy = cpufreq_cpu_get_new(0);
	policy->cpuinfo.max_freq = pll_l_val*HFPLL_FREQ_KHZ;

	printk(KERN_INFO "krait_oc: updating cpufreq tables\n");
	freq_table[12].frequency = pll_l_val*HFPLL_FREQ_KHZ;

	/* Save a pointer to the freq original table to restore if unloaded */
	orig_table = cpufreq_frequency_get_table(0);

	for_each_possible_cpu(cpu) {
		cpufreq_frequency_table_put_attr(cpu);
		cpufreq_frequency_table_get_attr(freq_table, cpu);
	}

	/* Index 20 is not used for scaling in the acpu_freq_tbl, so fill it
         * with our new freq. Change all three tables to account for all
	 * possible bins. */
	printk(KERN_INFO "krait_oc: updating nominal acpu_freq_tbl\n");
	acpu_freq_row_update(ACPU_FREQ_TBL_NOM_NAME, 20, pll_l_val, vdd_uv);
	printk(KERN_INFO "krait_oc: updating slow acpu_freq_tbl\n");
	acpu_freq_row_update(ACPU_FREQ_TBL_SLOW_NAME, 20, pll_l_val, vdd_uv);
	printk(KERN_INFO "krait_oc: updating fast acpu_freq_tbl\n");
	acpu_freq_row_update(ACPU_FREQ_TBL_FAST_NAME, 20, pll_l_val, vdd_uv);

	return 0;
}

static void __exit overclock_exit(void)
{
	struct cpufreq_policy *policy;
	uint cpu;

	if(kallsyms_lookup_name("cpufreq_cpu_get") != 0) {
		printk(KERN_INFO "krait_oc: reverting cpufreq policy\n");
		policy = cpufreq_cpu_get_new(0);
		policy->cpuinfo.max_freq = 1512000;

		printk(KERN_INFO "krait_oc: reverting cpufreq tables\n");
		for_each_possible_cpu(cpu) {
			cpufreq_frequency_table_put_attr(cpu);
			cpufreq_frequency_table_get_attr(orig_table, cpu);
		}
	}

	printk(KERN_INFO "krait_oc: unloaded\n");
}

module_init(overclock_init);
module_exit(overclock_exit);
