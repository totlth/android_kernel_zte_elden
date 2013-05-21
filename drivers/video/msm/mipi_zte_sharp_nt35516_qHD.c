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

#if 1
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/* 600*1024, RGB888, 3 Lane 55 fps video mode */
    /* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0x66, 0x26, 0x19, 0x00, 0x1c, 0x91, 0x1e, 0x8d,
	0x1c, 0x03, 0x04, 0xa0},
    /* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
    /* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x0, 0x89, 0x1, 0x1a, 0x00, 0x50, 0x48, 0x63,
	0x30, 0x07, 0x03,//0x41, 0x0f, 0x01,
	0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};
#else
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/* 600*1024, RGB888, 3 Lane 55 fps video mode */
    /* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0xab, 0x8a, 0x18, 0x00, 0x92, 0x97, 0x1b, 0x8c,
	0x0c, 0x03, 0x04, 0xa0},
    /* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
    /* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x0, 0x7f, 0x1, 0x1a, 0x00, 0x50, 0x48, 0x63,
	0x30, 0x07, 0x03,//0x41, 0x0f, 0x01,
	0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
};
#endif

static int bl_lpm;
static struct mipi_dsi_panel_platform_data *mipi_toshiba_pdata;


static struct dsi_buf toshiba_tx_buf;
static struct dsi_buf toshiba_rx_buf;

static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};


//*************************************
// Enable CMD 3
//*************************************
static char para1[5]={0xFF,0xAA,0x55,0x25,0x01};

// OSC = 30MHz
static char para2[5]={0xF3, 0x02, 0x03, 0x07, 0x15};

//*************************************
// Select CMD2,Page 0
//*************************************
static char para3[6]={0xF0,0x55,0xAA,0x52,0x08,0x00};

static char para31[3] = {0xb1, 0xD8,0x00};
// Source EQ
static char para4[5]={0xB8,0x01,0x02,0x02,0x02};

// Z Inversion
static char para5[4]={0xBC,0x05,0x05,0x05};

//*************************************
// Select CMD2,Page 1
//*************************************
static char para6[6]={0xF0,0x55,0xAA,0x52,0x08,0x01};

// AVDD: 6.0V
static char para7[4]={0xB0,0x05,0x05,0x05};

// AVEE: -6.0V
static char para8[4]={0xB1,0x05,0x05,0x05};

// AVDD: 2.5x VPNL
static char para9[4]={0xB6,0x44,0x44,0x44};

// AVEE: -2.5x VPNL
static char para10[4]={0xB7,0x34,0x34,0x34};

// VGLX: AVEE - AVDD + VCL
static char para11[4]={0xBA,0x24,0x24,0x24};

// VGMP: 4.7V,VGSP=0V
static char para12[4]={0xBC,0x00,0x88,0x00};

// VGMN: -4.7V,VGSN=-0V
static char para13[4]={0xBD,0x00,0x88,0x00};

// VCOM

static char para14[3]={0xbe, 0x00,0x50};


static struct dsi_cmd_desc toshiba_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};

static struct dsi_cmd_desc toshiba_display_on_cmds[] = {
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para1), para1},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para2), para2},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para3), para3},
		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para31),para31},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para4), para4},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para5), para5},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para6), para6},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para7), para7},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para8), para8},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para9), para9},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para10), para10},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para11), para11},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para12), para12},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para13), para13},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(para14),para14},
		//{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(para15),para15},
		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}

};
#if 0//pan
static struct dsi_cmd_desc toshiba_display_start[] = {
		{DTYPE_DCS_WRITE, 1, 0, 0, 150, sizeof(exit_sleep), exit_sleep},
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on}
};
#endif
static struct dsi_cmd_desc toshiba_display_stop[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};


static int mipi_toshiba_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	bool bSkip=true;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
#if 0//		
		mipi_dsi_cmds_tx(mfd, &toshiba_tx_buf, toshiba_display_start,
			ARRAY_SIZE(toshiba_display_start));
#endif
	if(bSkip)
	mipi_dsi_cmds_tx(mfd, &toshiba_tx_buf, toshiba_display_on_cmds,
			ARRAY_SIZE(toshiba_display_on_cmds));
printk("\n pan1111111111111");
	return 0;
}

static int mipi_toshiba_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	bool bSkip=false;
		
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
		
	mipi_dsi_cmds_tx(mfd, &toshiba_tx_buf, toshiba_display_stop,
			ARRAY_SIZE(toshiba_display_stop));
		
	if(bSkip)
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

		pinfo.xres = 540;
		pinfo.yres = 960;
		pinfo.type = MIPI_VIDEO_PANEL;
		pinfo.pdest = DISPLAY_1;
		pinfo.wait_cycle = 0;
		pinfo.bpp = 24;


#if 0	
		pinfo.lcdc.h_back_porch = 80;//100;
		pinfo.lcdc.h_front_porch = 20;
		pinfo.lcdc.h_pulse_width = 5;
		pinfo.lcdc.v_back_porch = 10;//10;//75;
		pinfo.lcdc.v_front_porch = 5;//5;
		pinfo.lcdc.v_pulse_width = 2;
#else//pan
		pinfo.lcdc.h_back_porch = 100;//100;
		pinfo.lcdc.h_front_porch = 100;
		pinfo.lcdc.h_pulse_width = 8;
		pinfo.lcdc.v_back_porch = 10;//10;//75;
		pinfo.lcdc.v_front_porch = 8;//5;
		pinfo.lcdc.v_pulse_width = 8;
#endif

		pinfo.lcdc.border_clr = 0;	/* blk */
		pinfo.lcdc.underflow_clr = 0xff;	/* blue */
		pinfo.lcdc.hsync_skew = 0;
		pinfo.bl_max = 100;
		pinfo.bl_min = 1;
		pinfo.fb_num = 2;
		pinfo.clk_rate = 384000000;
	
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

