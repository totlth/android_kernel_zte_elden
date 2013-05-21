/* include/linux/zte_hibernate.h
 *
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
 */


#ifndef _LINUX_ZTE_HIBERNATE_H
#define _LINUX_ZTE_HIBERNATE_H

#include <linux/notifier.h>
#include <linux/rtc.h>

#define ZTE_HIBERNATE_ON      1
#define ZTE_HIBERNATE_OFF      0

#ifdef CONFIG_ZTE_HIBERNATE

void alarm_wakeup_hibernate(void);
int register_hibernate_notifier(struct notifier_block *nb);
int unregister_hibernate_notifier(struct notifier_block *nb);
int is_in_hibernate(void);
void powerkey_press_hibernate(void);
void setup_hibernate_alarm(struct rtc_device *rtc, unsigned long timeout);
#else
static inline void alarm_wakeup_hibernate(void) {;}
static int inline register_hibernate_notifier(struct notifier_block *nb){return 0;}
static int inline unregister_hibernate_notifier(struct notifier_block *nb){return 0;}
static int inline is_in_hibernate(void){return 0;}
static inline void powerkey_press_hibernate(void) {;}
static inline void setup_hibernate_alarm(struct rtc_device *rtc, unsigned long timeout) {;}
#endif

#endif

