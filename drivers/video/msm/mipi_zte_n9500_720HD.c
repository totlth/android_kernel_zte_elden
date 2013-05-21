/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_toshiba.h"
#include <mach/gpio.h>


/* Macros assume PMIC GPIOs and MPPs start at 1 */
#define PM8921_GPIO_BASE		NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8921_GPIO_BASE)
#define PM8921_MPP_BASE			(PM8921_GPIO_BASE + PM8921_NR_GPIOS)
#define PM8921_MPP_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8921_MPP_BASE)
#define PM8921_IRQ_BASE			(NR_MSM_IRQS + NR_GPIO_IRQS)

/* #define DEBUG */
#define DEBUG
#ifdef DEBUG
#define DEV_DBG(args...)	printk(args)
#else
#define DEV_DBG(args...)	(void)0
#endif /* DEBUG */

/*#define NEED_SW_ESD*/
//#define NEED_SW_ESD
#ifdef NEED_SW_ESD
struct timer_list lcd_esd_timer;
struct workqueue_struct *lcd_esd_queue;
struct work_struct lcd_esd_work;
struct msm_fb_data_type *local_mfd=NULL;
static char timer_deleted = 0;
extern void lcd_reset_esd(void);
#endif
/*color enhance*/
#define COLOR_ENHANCE

static struct msm_panel_info pinfo;
//< 2012/6/13-N9500_add_new_lcd_bootloader_kernel-lizhiye- < short commond here >
extern u32 LcdPanleID;
//>2012/6/13-N9500_add_new_lcd_bootloader_kernel-lizhiye
extern int lcdinit_backlight_delay;
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
/* 720*1280, RGB888, 4 Lane 60  fps video mode */
	/* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0x6f, 0x18, 0x10, 0x00, 0x3b, 0x3f, 0x14, 0x1b,
	0x1b, 0x03, 0x04, 0xa0},
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x01, 0x84, 0x31, 0xda, 0x00, 0x5f, 0x48, 0x63,
	0x31, 0x0f, 0x03,//0x41, 0x0f, 0x01,
	0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};

static int bl_lpm;
static struct mipi_dsi_panel_platform_data *mipi_toshiba_pdata;
uint32_t mipi_get_icpanleid(struct msm_fb_data_type *mfd );
void mipi_read_otm_id(struct msm_fb_data_type *mfd);
static struct dsi_buf mipi_tx_buf;
static struct dsi_buf mipi_rx_buf;

static char OTM_CMI_panelid_rd_para[] = {0xA1, 0x00}; 

static struct dsi_cmd_desc OTM_CMI_panelid_rd_cmd = 
{
	DTYPE_GEN_READ1, 1, 0, 0, 1,sizeof(OTM_CMI_panelid_rd_para), OTM_CMI_panelid_rd_para
};
static char otm_cmd_oa[2] = {0x0a, 0x00};
static char otm_cmd_ob[2] = {0x0b, 0x00};
static struct dsi_cmd_desc get_otm1281_0x0a_value_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(otm_cmd_oa), otm_cmd_oa
};
static struct dsi_cmd_desc get_otm1281_0x0b_value_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(otm_cmd_ob), otm_cmd_ob
};

static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};




//truly OTM1281A
static char truly_cmd_FF1[] = {0xFF, 0x12,0x80,0x01};
static char truly_shift_FF2[] = {0x00, 0x80};
static char truly_cmd_FF2[] = {0xFF, 0x12,0x80};
static char truly_shift_b0[] = {0x00, 0xb0};
static char truly_cmd_b0[] = {0xb0, 0x20};
static char truly_shift_C51[] = {0x00, 0x90};
static char truly_cmd_C51[] = {0xC5, 0x20,0x6F,0x02,0x88,0x1d,0x15,0x00,0x08,0x44,0x44,0x44};
static char truly_shift_C52[] = {0x00, 0xa0};
static char truly_cmd_C52[] = {0xC5, 0x20,0x6F,0x02,0x88,0x1d,0x15,0x00,0x08};
static char truly_shift_C53[] = {0x00, 0x80};
static char truly_cmd_C53[] = {0xC5, 0x20,0x01,0x00,0xB0,0xB0,0x00,0x08,0x00};
static char truly_shift_D8[] = {0x00, 0x00};
static char truly_cmd_D8[] = {0xD8, 0x58,0x00,0x58,0x00};
static char truly_shift_F5[] = {0x00, 0xb8};
static char truly_cmd_F5[] = {0xF5, 0x0C,0x12};

static char truly_shift_C1_80[] = {0x00, 0x80};
static char truly_cmd_C1_80[] = {0xc1, 0x85};
static char truly_shift_C5_D0[] = {0x00, 0xD0};
static char truly_cmd_C5_D0[] = {0xc5,0x38};
#ifdef COLOR_ENHANCE
static char truly_shift_83[] = {0x00, 0x83};
static char truly_cmd_F4[] = {0xf4, 0x00};
static char truly_shift_00[] = {0x00, 0x00};
static char truly_cmd_D4[] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40};
static char truly_cmd_D5[] = {0xD5,0x00,0x60,0x00,0x60,0x00,0x5f,0x00,0x5f,0x00,
						  0x5e,0x00,0x5e,0x00,0x5d,0x00,0x5d,0x00,0x5d,0x00,
						  0x5c,0x00,0x5c,0x00,0x5b,0x00,0x5b,0x00,0x5a,0x00,
						  0x5a,0x00,0x5a,0x00,0x5b,0x00,0x5c,0x00,0x5d,0x00,
						  0x5d,0x00,0x5e,0x00,0x5f,0x00,0x60,0x00,0x61,0x00,
						  0x62,0x00,0x63,0x00,0x63,0x00,0x64,0x00,0x65,0x00,
						  0x66,0x00,0x67,0x00,0x68,0x00,0x69,0x00,0x69,0x00,
						  0x6a,0x00,0x6b,0x00,0x6c,0x00,0x6d,0x00,0x6e,0x00,
						  0x6f,0x00,0x6f,0x00,0x70,0x00,0x71,0x00,0x72,0x00,
						  0x73,0x00,0x74,0x00,0x74,0x00,0x75,0x00,0x76,0x00,
						  0x77,0x00,0x78,0x00,0x78,0x00,0x79,0x00,0x7a,0x00,
						  0x7b,0x00,0x7c,0x00,0x7d,0x00,0x7d,0x00,0x7e,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7e,0x00,0x7d,0x00,
						  0x7c,0x00,0x7b,0x00,0x7a,0x00,0x7a,0x00,0x79,0x00,
						  0x78,0x00,0x77,0x00,0x76,0x00,0x76,0x00,0x75,0x00,
						  0x74,0x00,0x73,0x00,0x72,0x00,0x71,0x00,0x71,0x00,
						  0x70,0x00,0x6f,0x00,0x6e,0x00,0x6d,0x00,0x6c,0x00,
						  0x6c,0x00,0x6b,0x00,0x6a,0x00,0x69,0x00,0x68,0x00,
						  0x67,0x00,0x66,0x00,0x66,0x00,0x66,0x00,0x65,0x00,
						  0x65,0x00,0x64,0x00,0x64,0x00,0x63,0x00,0x63,0x00,
						  0x63,0x00,0x62,0x00,0x62,0x00,0x61,0x00,0x61,0x00,
						  0x60};
#endif
static char truly_cmd_36[] = {0x36, 0xd0};
//static char truly_cmd_35[] = {0x35, 0x00};

static char truly_cmd_2c[] = {0x2c, 0x00};
static char truly_cmd_3c[] = {0x3c, 0x00};
static char truly_shift_B0_80[] = {0x00,0x80};
static char truly_cmd_B0_80[] = {0xb0,0x60};
static char truly_shift_B0_B1[] = {0x00,0xB1};
static char truly_cmd_B0_B1[] = {0xb0,004};




static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};


static struct dsi_cmd_desc truly_display_on_cmds[] = {
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_FF1),truly_cmd_FF1},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_FF2),truly_shift_FF2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_FF2),truly_cmd_FF2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_b0),truly_shift_b0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_b0),truly_cmd_b0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C51),truly_shift_C51},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C51),truly_cmd_C51},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C52),truly_shift_C52},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C52),truly_cmd_C52},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C53),truly_shift_C53},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C53),truly_cmd_C53},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_D8),truly_shift_D8},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D8),truly_cmd_D8},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_F5),truly_shift_F5},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F5),truly_cmd_F5},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C1_80),truly_shift_C1_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C1_80),truly_cmd_C1_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C5_D0),truly_shift_C5_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C5_D0),truly_cmd_C5_D0},


#ifdef COLOR_ENHANCE
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_83),truly_shift_83},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F4),truly_cmd_F4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D4),truly_cmd_D4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D5),truly_cmd_D5},

#endif
		{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(truly_cmd_36),truly_cmd_36},
//		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_35),truly_cmd_35},
		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep),exit_sleep},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on},

		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_2c),truly_cmd_2c},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_3c), truly_cmd_3c},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_shift_B0_80),truly_shift_B0_80},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_B0_80), truly_cmd_B0_80},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_shift_B0_B1),truly_shift_B0_B1},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_B0_B1), truly_cmd_B0_B1},

};


//< 2012/6/1-N9500_add_new_lcd_bootloader_kernel-lizhiye- < short commond here >
//cmi OTM1280A
static char cmi_cmd_FF[] = {0xFF, 0x12,0x80,0x01};
static char cmi_shift_FF_80[] = {0x00, 0x80};
static char cmi_cmd_FF_80[] = {0xFF, 0x12,0x80};
static char cmi_shift_B3_A0[] = {0x00,0xa0};
static char cmi_cmd_B3_A0[] = {0xb3,0x38,0x38};

static char cmi_cmd_truly_shift_b0[] = {0x00, 0xb0}; 
static char cmi_cmd_truly_cmd_b0[] = {0xb0, 0x20};

static char cmi_shift_CB_80[] = {0x00,0x80};
static char cmi_cmd_CB_80[] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_90[] = {0x00,0x90};
static char cmi_cmd_CB_90[] = {0xcb,0x00,0xc0,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_A0[] = {0x00,0xa0};
static char cmi_cmd_CB_A0[] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_B0[] = {0x00,0xb0};
static char cmi_cmd_CB_B0[] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_C0[] = {0x00,0xc0};
static char cmi_cmd_CB_C0[] = {0xcb,0x04,0x00,0x0f,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0xf4};
static char cmi_shift_CB_D0[] = {0x00,0xd0};
static char cmi_cmd_CB_D0[] = {0xcb,0xf4,0xf4,0x00,0xf4,0x08,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_E0[] = {0x00,0xe0};
static char cmi_cmd_CB_E0[] = {0xcb,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
static char cmi_shift_CB_F0[] = {0x00,0xf0};
static char cmi_cmd_CB_F0[] = {0xcb,0x00,0x70,0x01,0x00,0x00};
static char cmi_shift_CC_80[] = {0x00,0x80};
static char cmi_cmd_CC_80[] = {0xcc,0x41,0x42,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x52,0x55,0x43,0x53,0x65,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40};
static char cmi_shift_CC_A0[] = {0x00,0xA0};
static char cmi_cmd_CC_A0[] = {0xcc,0x41,0x42,0x47,0x48,0x4C,0x4B,0x4A,0x49,0x52,0x55,0x43,0x53,0x65,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0x01};
static char cmi_shift_CC_C0[] = {0x00,0xC0};
static char cmi_cmd_CC_C0[] = {0xcc,0x41,0x42,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x52,0x55,0x43,0x53,0x54,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40};
static char cmi_shift_CC_E0[] = {0x00,0xE0};
static char cmi_cmd_CC_E0[] = {0xcc,0x41,0x42,0x47,0x48,0x4C,0x4B,0x4A,0x49,0x52,0x55,0x43,0x53,0x54,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0x01};
static char cmi_shift_C1_90[] = {0x00,0x90};
static char cmi_cmd_C1_90[] = {0xc1,0x22,0x00,0x00,0x00,0x00};
static char cmi_shift_C0_80[] = {0x00,0x80};
static char cmi_cmd_C0_80[] = {0xc0,0x00,0x87,0x00,0x06,0x0a,0x00,0x87,0x06,0x0a,0x00,0x00,0x00};
static char cmi_shift_C0_90[] = {0x00,0x90};
static char cmi_cmd_C0_90[] = {0xc0,0x00,0x0a,0x00,0x14,0x00,0x2a};
static char cmi_shift_C0_A0[] = {0x00,0xA0};
static char cmi_cmd_C0_A0[] = {0xc0,0x00,0x03,0x01,0x01,0x01,0x01,0x1a,0x03,0x00,0x02};
static char cmi_shift_C2_80[] = {0x00,0x80};
static char cmi_cmd_C2_80[] = {0xc2,0x03,0x02,0x00,0x00,0x00,0x02,0x00,0x22};
static char cmi_shift_C2_90[] = {0x00,0x90};
static char cmi_cmd_C2_90[] = {0xc2,0x03,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x22};
static char cmi_shift_C2_A0[] = {0x00,0xA0};
static char cmi_cmd_C2_A0[] = {0xc2,0xff,0x00,0xff,0x00,0x00,0x0a,0x00,0x0a};
static char cmi_shift_C2_B0[] = {0x00,0xB0};
static char cmi_cmd_C2_B0[] = {0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_C2_C0[] = {0x00,0xC0};
static char cmi_cmd_C2_C0[] = {0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_C2_E0[] = {0x00,0xE0};
static char cmi_cmd_C2_E0[] = {0xc2,0x84,0x00,0x10,0x0d};
static char cmi_shift_C2_B3[] = {0x00,0xB3};
static char cmi_cmd_C2_B3[] = {0xc0,0x0f};
static char cmi_shift_C1_A2[] = {0x00,0xA2};
static char cmi_cmd_C1_A2[] = {0xc1,0xff};
static char cmi_shift_C0_B4[] = {0x00,0xB4};
static char cmi_cmd_C0_B4[] = {0xc0,0x54,0x00};
static char cmi_shift_C5_80[] = {0x00,0x80};
static char cmi_cmd_C5_80[] = {0xc5,0x20,0x07,0x00,0xb0,0xb0,0x00,0x08,0x00};
static char cmi_shift_C5_90[] = {0x00,0x90};
static char cmi_cmd_C5_90[] = {0xc5,0x20,0x85,0x02,0x88,0x96,0x15,0x00,0x08,0x44,0x44,0x44};

static char cmi_shift_D8_00[] = {0x00,0x00};
static char cmi_cmd_D8_00[] = {0xd8,0x52,0x00,0x52,0x00};
static char cmi_shift_D9_00[] = {0x00,0x00};
static char cmi_cmd_D9_00[] = {0xd9,0x8f,0x73,0x80};
static char cmi_shift_C0_C0[] = {0x00,0xC0};
static char cmi_cmd_C0_C0[] = {0xc0,0x95};
static char cmi_shift_C0_D0[] = {0x00,0xD0};
static char cmi_cmd_C0_D0[] = {0xc0,0x05};
static char cmi_shift_F5_B6[] = {0x00,0xB6};
static char cmi_cmd_F5_B6[] = {0xf5,0x00,0x00};
static char cmi_shift_B3_B0[] = {0x00,0xB0};
static char cmi_cmd_B3_B0[] = {0xb3,0x11};
static char cmi_shift_F5_B0[] = {0x00,0xB0};
static char cmi_cmd_F5_B0[] = {0xf5,0x00,0x20};
static char cmi_shift_F5_B8[] = {0x00,0xB8};
static char cmi_cmd_F5_B8[] = {0xf5,0x0c,0x12};
static char cmi_shift_F5_94[] = {0x00,0x94};
static char cmi_cmd_F5_94[] = {0xf5,0x0a,0x14,0x06,0x17};
static char cmi_shift_F5_A2[] = {0x00,0xA2};
static char cmi_cmd_F5_A2[] = {0xf5,0x0a,0x14,0x07,0x14};
static char cmi_shift_F5_90[] = {0x00,0x90};
static char cmi_cmd_F5_90[] = {0xf5,0x07,0x16,0x07,0x14};
static char cmi_shift_F5_A0[] = {0x00,0xA0};
static char cmi_cmd_F5_A0[] = {0xf5,0x02,0x12,0x0a,0x12,0x07,0x12,0x06,0x12,0x0b,0x12,0x08,0x12};
static char cmi_cmd_36[] = {0x36,0xd1};
static char cmi_shift_D6_80[] = {0x00,0x80};
static char cmi_cmd_D6_80[] = {0xd6,0x00};
static char cmi_shift_E1_00[] = {0x00,0x00};
static char cmi_cmd_E1_00[] = {0xE1,0x2C,0x2F,0x36,0x3E,0x0B,0x05,0x14,0x09,0x07,0x08,0x09,0x1C,0x05,0x0B,0x11,0x0E,0x0B,0x0B};
static char cmi_shift_E2_00[] = {0x00,0x00};
static char cmi_cmd_E2_00[] = {0xE2,0x2C,0x2F,0x36,0x3E,0x0B,0x05,0x14,0x09,0x07,0x08,0x09,0x1C,0x05,0x0B,0x11,0x0E,0x0B,0x0B};
static char cmi_shift_E3_00[] = {0x00,0x00};
static char cmi_cmd_E3_00[] = {0xE3,0x2C,0x2E,0x35,0x3C,0x0D,0x06,0x16,0x09,0x07,0x08,0x0A,0x1A,0x05,0x0B,0x12,0x0E,0x0B,0x0B};
static char cmi_shift_E4_00[] = {0x00,0x00};
static char cmi_cmd_E4_00[] = {0xE4,0x2C,0x2E,0x35,0x3C,0x0D,0x06,0x16,0x09,0x07,0x08,0x0A,0x1A,0x05,0x0B,0x12,0x0E,0x0B,0x0B};
static char cmi_shift_E5_00[] = {0x00,0x00};
static char cmi_cmd_E5_00[] = {0xE5,0x0E,0x16,0x23,0x2E,0x0E,0x07,0x1C,0x0A,0x08,0x07,0x09,0x19,0x05,0x0C,0x11,0x0D,0x0B,0x0B};
static char cmi_shift_E6_00[] = {0x00,0x00};
static char cmi_cmd_E6_00[] = {0xE6,0x0E,0x16,0x23,0x2E,0x0E,0x07,0x1C,0x0A,0x08,0x07,0x09,0x19,0x05,0x0C,0x11,0x0D,0x0B,0x0B};

static char cmi_shift_2A_4P[]={0x00, 0x00};
static char cmi_cmd_2A_4P[]={0x2A, 0x00, 0x00, 0x02, 0xcf};
static char cmi_shift_2B_4P[]={0x00, 0x00};
static char cmi_cmd_2B_4P[]={0x2b, 0x00, 0x00, 0x04, 0xff};
static char cmi_shift_C1_80[] = {0x00, 0x80};
static char cmi_cmd_C1_80[] = {0xc1, 0x85};
static char cmi_shift_C5_D0[] = {0x00, 0xD0};
static char cmi_cmd_C5_D0[] = {0xc5,0x38};

static struct dsi_cmd_desc cmi_display_on_cmds[] = {
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_FF),cmi_cmd_FF},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_FF_80),cmi_shift_FF_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_FF_80),cmi_cmd_FF_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_B3_A0),cmi_shift_B3_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_B3_A0),cmi_cmd_B3_A0},

		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_truly_shift_b0),cmi_cmd_truly_shift_b0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_truly_cmd_b0),cmi_cmd_truly_cmd_b0},
		
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_80),cmi_shift_CB_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_80),cmi_cmd_CB_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_90),cmi_shift_CB_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_90),cmi_cmd_CB_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_A0),cmi_shift_CB_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_A0),cmi_cmd_CB_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_B0),cmi_shift_CB_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_B0),cmi_cmd_CB_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_C0),cmi_shift_CB_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_C0),cmi_cmd_CB_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_D0),cmi_shift_CB_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_D0),cmi_cmd_CB_D0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_E0),cmi_shift_CB_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_E0),cmi_cmd_CB_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_F0),cmi_shift_CB_F0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_F0),cmi_cmd_CB_F0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_80),cmi_shift_CC_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_80),cmi_cmd_CC_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_A0),cmi_shift_CC_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_A0),cmi_cmd_CC_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_C0),cmi_shift_CC_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_C0),cmi_cmd_CC_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_E0),cmi_shift_CC_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_E0),cmi_cmd_CC_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_90),cmi_shift_C1_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_90),cmi_cmd_C1_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_80),cmi_shift_C0_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_80),cmi_cmd_C0_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_90),cmi_shift_C0_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_90),cmi_cmd_C0_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_A0),cmi_shift_C0_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_A0),cmi_cmd_C0_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_80),cmi_shift_C2_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_80),cmi_cmd_C2_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_90),cmi_shift_C2_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_90),cmi_cmd_C2_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_A0),cmi_shift_C2_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_A0),cmi_cmd_C2_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_B0),cmi_shift_C2_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_B0),cmi_cmd_C2_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_C0),cmi_shift_C2_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_C0),cmi_cmd_C2_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_E0),cmi_shift_C2_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_E0),cmi_cmd_C2_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_B3),cmi_shift_C2_B3},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_B3),cmi_cmd_C2_B3},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_A2),cmi_shift_C1_A2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_A2),cmi_cmd_C1_A2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_B4),cmi_shift_C0_B4},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_B4),cmi_cmd_C0_B4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_80),cmi_shift_C5_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_80),cmi_cmd_C5_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_90),cmi_shift_C5_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_90),cmi_cmd_C5_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D8_00),cmi_shift_D8_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_D8_00),cmi_cmd_D8_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D9_00),cmi_shift_D9_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_D9_00),cmi_cmd_D9_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_C0),cmi_shift_C0_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_C0),cmi_cmd_C0_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_D0),cmi_shift_C0_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_D0),cmi_cmd_C0_D0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B6),cmi_shift_F5_B6},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B6),cmi_cmd_F5_B6},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_B3_B0),cmi_shift_B3_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_B3_B0),cmi_cmd_B3_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B0),cmi_shift_F5_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B0),cmi_cmd_F5_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B8),cmi_shift_F5_B8},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B8),cmi_cmd_F5_B8},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_94),cmi_shift_F5_94},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_94),cmi_cmd_F5_94},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_A2),cmi_shift_F5_A2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_A2),cmi_cmd_F5_A2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_90),cmi_shift_F5_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_90),cmi_cmd_F5_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_A0),cmi_shift_F5_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_A0),cmi_cmd_F5_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_36),cmi_cmd_36},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D6_80),cmi_shift_D6_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_D6_80),cmi_cmd_D6_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E1_00),cmi_shift_E1_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E1_00),cmi_cmd_E1_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E2_00),cmi_shift_E2_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E2_00),cmi_cmd_E2_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E3_00),cmi_shift_E3_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E3_00),cmi_cmd_E3_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E4_00),cmi_shift_E4_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E4_00),cmi_cmd_E4_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E5_00),cmi_shift_E5_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E5_00),cmi_cmd_E5_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E6_00),cmi_shift_E6_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E6_00),cmi_cmd_E6_00},
		
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_2A_4P),cmi_shift_2A_4P},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_2A_4P), cmi_cmd_2A_4P},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_2B_4P),cmi_shift_2B_4P},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_2B_4P), cmi_cmd_2B_4P},

		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_80),cmi_shift_C1_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_80),cmi_cmd_C1_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_D0),cmi_shift_C5_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_D0),cmi_cmd_C5_D0},
#ifdef COLOR_ENHANCE
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_83),truly_shift_83},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F4),truly_cmd_F4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D4),truly_cmd_D4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D5),truly_cmd_D5},

#endif	
		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep),exit_sleep},
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on},
};
//>2012/6/1-N9500_add_new_lcd_bootloader_kernel-lizhiye



//< 2012/6/13-N9500_add_new_lcd_bootloader_kernel-lizhiye- < short commond here >
static void lcd_panle_reset(void)
{
	int gpio_rst;
	gpio_rst = PM8921_GPIO_PM_TO_SYS(43);
	gpio_set_value_cansleep(gpio_rst, 1); /* disp enable */
	msleep(10);
	gpio_set_value_cansleep(gpio_rst, 0); /* disp enable */
	msleep(10);
	gpio_set_value_cansleep(gpio_rst, 1); /* disp enable */
	msleep(40); //200
}

static int firsttimeboot = 1;
static int mipi_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	printk("LCD, mipi_lcd_on , LcdPanleID = %d\n", LcdPanleID);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
	if(firsttimeboot){
		firsttimeboot = 0;
		printk("LCD, first time run!!!!\n");
		if(LcdPanleID != LCD_PANEL_NOPANEL)
			return 0;
		else
			LcdPanleID = mipi_get_icpanleid(mfd);
	}
	
	lcd_panle_reset();
	mipi_set_tx_power_mode(1);

	switch(LcdPanleID)
	{				
		case LCD_PANEL_4P3_OTM1280_OT_CMI:	//310
			printk("LCD: LCD_PANEL_4P3_OTM1280_OT_CMI\n");
			mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, cmi_display_on_cmds, ARRAY_SIZE(cmi_display_on_cmds));
			break;
		case LCD_PANEL_4P3_OTM1281_OT_TRULY:	//311
			printk("LCD: LCD_PANEL_4P3_OTM1281_OT_TRULY\n");
			mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, truly_display_on_cmds, ARRAY_SIZE(truly_display_on_cmds));
			mipi_read_otm_id(mfd);
			break;		
		default:
			printk("LCD: not found the lcd id, error\n");
			break;
	}
	mipi_set_tx_power_mode(0);

	return 0;
}

static int mipi_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	
	printk("\nLCD %s: \n", __func__);
	
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
	{
		printk("\nLCD %s no such device: \n", __func__);	
		return -ENODEV;
	}
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, display_off_cmds,
			ARRAY_SIZE(display_off_cmds));

	return 0;
}

static bool onewiremode = true;
static void select_1wire_mode(void)
{
	gpio_direction_output(bl_lpm, 1);
	udelay(120);
	gpio_direction_output(bl_lpm, 0);
	udelay(280);				////ZTE_LCD_LUYA_20100226_001
	gpio_direction_output(bl_lpm, 1);
	udelay(650);				////ZTE_LCD_LUYA_20100226_001
	
}

static void send_bkl_address(void)
{
	unsigned int i,j;
	i = 0x72;
	gpio_direction_output(bl_lpm, 1);
	udelay(10);
	printk("[LY] send_bkl_address \n");
	for(j = 0; j < 8; j++)
	{
		if(i & 0x80)
		{
			gpio_direction_output(bl_lpm, 0);
			udelay(10);
			gpio_direction_output(bl_lpm, 1);
			udelay(180);
		}
		else
		{
			gpio_direction_output(bl_lpm, 0);
			udelay(180);
			gpio_direction_output(bl_lpm, 1);
			udelay(10);
		}
		i <<= 1;
	}
	gpio_direction_output(bl_lpm, 0);
	udelay(10);
	gpio_direction_output(bl_lpm, 1);

}

static void send_bkl_data(int level)
{
	unsigned int i,j;
	i = level & 0x1F;
	gpio_direction_output(bl_lpm, 1);
	udelay(10);
	printk("[LY] send_bkl_data \n");
	for(j = 0; j < 8; j++)
	{
		if(i & 0x80)
		{
			gpio_direction_output(bl_lpm, 0);
			udelay(10);
			gpio_direction_output(bl_lpm, 1);
			udelay(180);
		}
		else
		{
			gpio_direction_output(bl_lpm, 0);
			udelay(180);
			gpio_direction_output(bl_lpm, 1);
			udelay(10);
		}
		i <<= 1;
	}
	gpio_direction_output(bl_lpm, 0);
	udelay(10);
	gpio_direction_output(bl_lpm, 1);

}

static void mipi_set_backlight(struct msm_fb_data_type *mfd)
{
       /*value range is 1--32*/
    int current_lel = mfd->bl_level;
    unsigned long flags;


    printk("[ZYF] lcdc_set_bl level=%d, %d\n", 
		   current_lel , mfd->panel_power_on);

    if(!mfd->panel_power_on)
	  {
    	gpio_direction_output(bl_lpm, 0);			///ZTE_LCD_LUYA_20100201_001
			onewiremode = FALSE;
	    return;
    }

    if(current_lel < 1)
    {
        current_lel = 0;
    }
    if(current_lel > 32)
    {
        current_lel = 32;
    }

    /*ZTE_BACKLIGHT_WLY_005,@2009-11-28, set backlight as 32 levels, end*/
    local_irq_save(flags);
    if(current_lel==0)
    {
    	gpio_direction_output(bl_lpm, 0);
		mdelay(3);
		onewiremode = FALSE;
			
    }
    else 
	{
		if(!onewiremode)	//select 1 wire mode
		{
			printk("[LY] before select_1wire_mode\n");
			select_1wire_mode();
			onewiremode = TRUE;
		}
		send_bkl_address();
		send_bkl_data(current_lel-1);

	}
    local_irq_restore(flags);
}

static uint32 mipi_read_commic_reg(struct msm_fb_data_type *mfd,struct dsi_cmd_desc *para,uint32 len,int mode)
{
	uint32 panelid = 0;
	mipi_dsi_buf_init(&mipi_tx_buf);
	mipi_dsi_buf_init(&mipi_rx_buf);
	mipi_dsi_cmd_bta_sw_trigger(); 
	if(mode)
		mipi_set_tx_power_mode(1);
	else 
		mipi_set_tx_power_mode(0);
	mipi_dsi_cmds_rx(mfd,&mipi_tx_buf, &mipi_rx_buf, para,len);
	if(mode)
		mipi_set_tx_power_mode(0);
	panelid = *(uint32 *)(mipi_rx_buf.data);
	return panelid;
}
void mipi_read_otm_id(struct msm_fb_data_type *mfd)
{
	uint32_t reg1 = 0,reg2 = 0;
	reg1 =  mipi_read_commic_reg(mfd,&get_otm1281_0x0a_value_cmd, 1,1);
	reg2 =  mipi_read_commic_reg(mfd,&get_otm1281_0x0b_value_cmd, 1,1);
	printk("debug read reg[0xa]= [%x]   reg[0xb] = [%x]\n",reg1&0xff,reg2&0xff);
	if(((reg1&0xff)!=0x9c)&&((reg2&0xff)!=0xd0))
		pr_err("%s  failed \n", __func__);
	
}
uint32_t mipi_get_icpanleid(struct msm_fb_data_type *mfd )
{
	uint32_t panleid = 0;
	lcd_panle_reset();
	panleid =  mipi_read_commic_reg(mfd,&OTM_CMI_panelid_rd_cmd, 2,1);
	printk("debug read panleid is %x\n",panleid & 0xffffffff);
	switch(panleid & 0xffff)
	{
		case 0x8012:
			printk("lizhiye, mipi_get_icpanleid, LCD_PANEL_4P3_OTM1280_OT_CMI = %d\n", LCD_PANEL_4P3_OTM1280_OT_CMI);
			return LCD_PANEL_4P3_OTM1280_OT_CMI;	//cmi
		case 0x8112:
			printk("lizhiye, mipi_get_icpanleid, LCD_PANEL_4P3_OTM1281_OT_TRULY = %d\n", LCD_PANEL_4P3_OTM1281_OT_TRULY);
			return LCD_PANEL_4P3_OTM1281_OT_TRULY;	//ovi, truly
		default:
			break;
	}
	return 0;
}
static int __devinit mipi_lcd_probe(struct platform_device *pdev)
{
	if (pdev->id == 0) {
		mipi_toshiba_pdata = pdev->dev.platform_data;
		return 0;
	}

	if (mipi_toshiba_pdata == NULL) {
		pr_err("%s.invalid platform data.\n", __func__);
		return -ENODEV;
	}

	if (mipi_toshiba_pdata != NULL)
		bl_lpm = mipi_toshiba_pdata->gpio[0];
		//gpio_request(bl_lpm,"backlight");
	lcdinit_backlight_delay = 60;
	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_lcd_probe,
	.driver = {
		.name   = "mipi_toshiba",
	},
};

static struct msm_fb_panel_data toshiba_panel_data = {
	.on		= mipi_lcd_on,
	.off		= mipi_lcd_off,
	.set_backlight  = mipi_set_backlight,
};

static int ch_used[3];

int mipi_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_toshiba", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	toshiba_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &toshiba_panel_data,
		sizeof(toshiba_panel_data));
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init mipi_lcd_init(void)
{
	mipi_dsi_buf_alloc(&mipi_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&mipi_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_lcd_init);

//pinfo.clk_rate = 384000000;
static int __init mipi_video_wsvga_pt_init(void)
{
		int ret;

		pinfo.xres = 720;
		pinfo.yres = 1280;
		pinfo.type = MIPI_VIDEO_PANEL;
		pinfo.pdest = DISPLAY_1;
		pinfo.wait_cycle = 0;
		pinfo.bpp = 24;


		pinfo.lcdc.h_back_porch = 70;//100;
		pinfo.lcdc.h_front_porch = 20;//50;
		pinfo.lcdc.h_pulse_width =   8;
		pinfo.lcdc.v_back_porch = 20;//24;//10;//75;
		pinfo.lcdc.v_front_porch = 20;//60;//5;
		pinfo.lcdc.v_pulse_width =  2;


		pinfo.lcdc.border_clr = 0;	/* blk */
		pinfo.lcdc.underflow_clr = 0xff;	/* blue */
		pinfo.lcdc.hsync_skew = 0;
		pinfo.bl_max = 30;
		pinfo.bl_min = 1;
		pinfo.fb_num = 2;

	
		pinfo.mipi.mode = DSI_VIDEO_MODE;
		pinfo.mipi.pulse_mode_hsa_he = TRUE;
		pinfo.mipi.hfp_power_stop = TRUE;
		pinfo.mipi.hbp_power_stop = TRUE;
		pinfo.mipi.hsa_power_stop = TRUE;
		pinfo.mipi.eof_bllp_power_stop = TRUE;
		pinfo.mipi.bllp_power_stop = TRUE;
		pinfo.mipi.traffic_mode = DSI_NON_BURST_SYNCH_PULSE;
		pinfo.mipi.dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
		pinfo.mipi.vc = 0;
		pinfo.mipi.rgb_swap = DSI_RGB_SWAP_BGR;
		pinfo.mipi.data_lane0 = TRUE;
		pinfo.mipi.data_lane1 = TRUE;
		pinfo.mipi.data_lane2 = TRUE;
		pinfo.mipi.data_lane3 = TRUE;
		pinfo.mipi.t_clk_post = 0x03;
		pinfo.mipi.t_clk_pre = 0x24;
		pinfo.mipi.stream = 0; /* dma_p */
		pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
		pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
		pinfo.mipi.frame_rate =60;
		pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;


	ret = mipi_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_WVGA_PT);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_wsvga_pt_init);

