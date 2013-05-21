/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
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

#include "mipi_otm_hd.h"

extern char Lcd_display_mode;

static struct msm_panel_info pinfo;


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

static int __init mipi_otm_video_hd_pt_init(void)
{
	int ret;

	if (Lcd_display_mode)
		return 0;

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
		pinfo.mdp_dsi_calibration = mdp_dsi_calibration;
		pinfo.is_sw_resolve_esd_func = otm1281_esd_video_func;
	ret = mipi_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_QHD_PT);
	if (ret)
		pr_err("%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_otm_video_hd_pt_init);
