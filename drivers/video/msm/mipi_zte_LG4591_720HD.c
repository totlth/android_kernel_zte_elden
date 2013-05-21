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

static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/* 600*1024, RGB888, 3 Lane 55 fps video mode */
    /* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0x66, 0x26, 0x0c, 0x00, 0x0d, 0x82, 0x1e, 0x87,
	0x0d, 0x03, 0x04, 0xa0},
    /* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
    /* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x0, 0x7A, 0x1, 0x1a, 0x00, 0x50, 0x48, 0x63,
	0x30, 0x07, 0x03,//0x41, 0x0f, 0x01,
	0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};

static int bl_lpm;
static struct mipi_dsi_panel_platform_data *mipi_toshiba_pdata;


static struct dsi_buf toshiba_tx_buf;
static struct dsi_buf toshiba_rx_buf;

//static char exit_sleep[2] = {0x11, 0x00};
//static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};



static char cmd_E0[] = {0xE0, 0x43, 0x40, 0x80, 0x00, 0x00};
static char cmd_B5[] = {0xB5, 0x14, 0x20, 0x40, 0x00, 0x00};
static char cmd_B6[] = {0xB6, 0x01, 0x16, 0x0F, 0x16, 0x13}; 
static char cmd_D0[] = {0xD0, 0x00, 0x66, 0x76, 0x04, 0x02, 0x02, 0x42, 0x02, 0x03};
static char cmd_D1[] = {0xD1, 0x00, 0x66, 0x76, 0x04, 0x02, 0x02, 0x42, 0x02, 0x03}; 
static char cmd_D2[] = {0xD2, 0x00, 0x66, 0x76, 0x04, 0x02, 0x02, 0x42, 0x02, 0x03};
static char cmd_D3[] = {0xD3, 0x00, 0x66, 0x76, 0x04, 0x02, 0x02, 0x42, 0x02, 0x03};
static char cmd_D4[] = {0xD4, 0x00, 0x66, 0x76, 0x04, 0x02, 0x02, 0x42, 0x02, 0x03};
static char cmd_D5[] = {0xD5, 0x00, 0x66, 0x76, 0x04, 0x02, 0x02, 0x42, 0x02, 0x03};
//500
static char cmd_C0[] = {0xC0, 0x00, 0x00};
static char cmd_C3[] = {0xC3, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x67, 0x88, 0x34, 0x02}; 
static char cmd_C4[] = {0xC4, 0x22, 0x24, 0x19, 0x15, 0x59}; 
static char cmd_F9[] = {0xF9, 0x00, 0x00}; 
static char cmd_11[] = {0x11, 0x00};
//500
static char cmd_29[] = {0x29, 0x00};



static struct dsi_cmd_desc toshiba_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};

static struct dsi_cmd_desc toshiba_display_on_cmds[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_E0),cmd_E0},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_B5),cmd_B5},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_B6),cmd_B6},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_D0),cmd_D0},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_D1),cmd_D1},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_D2),cmd_D2},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_D3),cmd_D3},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_D4),cmd_D4},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 500, sizeof(cmd_D5),cmd_D5},
		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_C0),cmd_C0},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_C3),cmd_C3},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_C4),cmd_C4},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(cmd_F9),cmd_F9},
		{DTYPE_DCS_WRITE, 1, 0, 0, 500, sizeof(cmd_11),cmd_11},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 1, sizeof(cmd_29), cmd_29},
};


static int mipi_toshiba_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(mfd, &toshiba_tx_buf, toshiba_display_on_cmds,
			ARRAY_SIZE(toshiba_display_on_cmds));

	return 0;
}

static int mipi_toshiba_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(mfd, &toshiba_tx_buf, toshiba_display_off_cmds,
			ARRAY_SIZE(toshiba_display_off_cmds));

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

static void mipi_toshiba_set_backlight(struct msm_fb_data_type *mfd)
{
       /*value range is 1--32*/
    int current_lel = mfd->bl_level;
    unsigned long flags;


    printk("[ZYF] lcdc_set_bl level=%d, %d\n", 
		   current_lel , mfd->panel_power_on);

//    if(!mfd->panel_power_on)
//	{
//    	gpio_direction_output(bl_lpm, 0);			///ZTE_LCD_LUYA_20100201_001
//	    return;
//    }

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

static int __devinit mipi_toshiba_lcd_probe(struct platform_device *pdev)
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

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_toshiba_lcd_probe,
	.driver = {
		.name   = "mipi_toshiba",
	},
};

static struct msm_fb_panel_data toshiba_panel_data = {
	.on		= mipi_toshiba_lcd_on,
	.off		= mipi_toshiba_lcd_off,
	.set_backlight  = mipi_toshiba_set_backlight,
};

static int ch_used[3];

int mipi_toshiba_device_register(struct msm_panel_info *pinfo,
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

static int __init mipi_toshiba_lcd_init(void)
{
	mipi_dsi_buf_alloc(&toshiba_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&toshiba_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_toshiba_lcd_init);

//pinfo.clk_rate = 384000000;
static int __init mipi_video_toshiba_wsvga_pt_init(void)
{
		int ret;

		pinfo.xres = 720;
		pinfo.yres = 1280;
		pinfo.type = MIPI_VIDEO_PANEL;
		pinfo.pdest = DISPLAY_1;
		pinfo.wait_cycle = 0;
		pinfo.bpp = 24;
	
		pinfo.lcdc.h_back_porch = 150;//100;
		pinfo.lcdc.h_front_porch = 70;//50;
		pinfo.lcdc.h_pulse_width = 5;
		pinfo.lcdc.v_back_porch = 10;//10;//75;
		pinfo.lcdc.v_front_porch = 5;//5;
		pinfo.lcdc.v_pulse_width = 1;
	

		pinfo.lcdc.border_clr = 0;	/* blk */
		pinfo.lcdc.underflow_clr = 0xff;	/* blue */
		pinfo.lcdc.hsync_skew = 0;
		pinfo.bl_max = 100;
		pinfo.bl_min = 1;
		pinfo.fb_num = 2;
		//pinfo.clk_rate = 384000000;
	
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
		pinfo.mipi.data_lane1 = true;
		pinfo.mipi.data_lane2 = true;
		pinfo.mipi.data_lane3 = true;
		pinfo.mipi.t_clk_post = 0x03;
		pinfo.mipi.t_clk_pre = 0x24;
		pinfo.mipi.stream = 0; /* dma_p */
		pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW;
		pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
		pinfo.mipi.frame_rate = 60;
		pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;


	ret = mipi_toshiba_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_WVGA_PT);
	if (ret)
		printk(KERN_ERR "%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_toshiba_wsvga_pt_init);

