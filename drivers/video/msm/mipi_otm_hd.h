/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef MIPI_ZTE_LCD
#define MIPI_ZTE_LCD

#include "msm_fb.h"
#include "mipi_dsi.h"
#include <mach/gpio.h>


extern int lcdinit_backlight_delay;

extern char Lcd_display_mode;



int mipi_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel);
uint32_t mipi_get_icpanleid(struct msm_fb_data_type *mfd );

void mdp_dsi_calibration(void);

void	mipi_tx_init_to_lcd(struct msm_fb_data_type *mfd);

void	lcd_init_prepare(u32 LcdPanleID);
	
uint32 mipi_read_commic_reg(struct msm_fb_data_type *mfd,struct dsi_cmd_desc *para,uint32 len,int mode);

void lcd_panle_reset(void);
uint8 otm1281_esd_video_func(struct msm_fb_data_type *mfd) ;

void mipi_set_backlight(struct msm_fb_data_type *mfd);

 void lcd_esd_work_func(struct work_struct *work);

  void lcd_esd_state_timer(unsigned long data);
  
/* #define DEBUG */
//#define DEBUG
#ifdef DEBUG
#define DEV_DBG(args...)	printk(args)
#else
#define DEV_DBG(args...)	(void)0
#endif /* DEBUG */




#endif  /* MIPI_ZTE_LCD */
