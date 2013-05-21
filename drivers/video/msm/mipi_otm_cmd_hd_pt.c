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

static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_cmd_mode_phy_db = {
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
 void mdp_dsi_calibration(void)
{
printk("mdp_dsi_calibration\n");
	//ck
MIPI_OUTP(MIPI_DSI_BASE + 0x400, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x404, 0x67);
MIPI_OUTP(MIPI_DSI_BASE + 0x408, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x40c, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x414, 0x01);
MIPI_OUTP(MIPI_DSI_BASE + 0x418, 0x88);
//ln0
MIPI_OUTP(MIPI_DSI_BASE + 0x300, 0x80);
MIPI_OUTP(MIPI_DSI_BASE + 0x304, 0x45);
MIPI_OUTP(MIPI_DSI_BASE + 0x308, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x30c, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x314, 0x01);
MIPI_OUTP(MIPI_DSI_BASE + 0x318, 0x66);
//ln1
MIPI_OUTP(MIPI_DSI_BASE + 0x340, 0x80);
MIPI_OUTP(MIPI_DSI_BASE + 0x344, 0x45);
MIPI_OUTP(MIPI_DSI_BASE + 0x348, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x34c, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x354, 0x01);
MIPI_OUTP(MIPI_DSI_BASE + 0x358, 0x66);
//ln2
MIPI_OUTP(MIPI_DSI_BASE + 0x380, 0x80);
MIPI_OUTP(MIPI_DSI_BASE + 0x384, 0x45);
MIPI_OUTP(MIPI_DSI_BASE + 0x388, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x38c, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x394, 0x01);
MIPI_OUTP(MIPI_DSI_BASE + 0x398, 0x66);
//ln3
MIPI_OUTP(MIPI_DSI_BASE + 0x3c0, 0x80);
MIPI_OUTP(MIPI_DSI_BASE + 0x3c4, 0x45);
MIPI_OUTP(MIPI_DSI_BASE + 0x3c8, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x3cc, 0x00);
MIPI_OUTP(MIPI_DSI_BASE + 0x3d4, 0x01);
MIPI_OUTP(MIPI_DSI_BASE + 0x3d8, 0x66);
}
static int __init mipi_otm_cmd_hd_pt_init(void)
{
	int ret;

	if (!Lcd_display_mode)
		return 0;

	pinfo.xres = 720;
	pinfo.yres = 1280;
	pinfo.type = MIPI_CMD_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.lcdc.h_back_porch = 50;
	pinfo.lcdc.h_front_porch = 50;
	pinfo.lcdc.h_pulse_width = 20;
	pinfo.lcdc.v_back_porch = 11;
	pinfo.lcdc.v_front_porch = 10;
	pinfo.lcdc.v_pulse_width = 5;
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.bl_max = 30;
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 389005096;
	pinfo.is_3d_panel = FB_TYPE_3D_PANEL;
	pinfo.lcd.vsync_enable = TRUE;
	pinfo.lcd.hw_vsync_mode = TRUE;
	pinfo.lcd.refx100 = 6100; /* adjust refx100 to prevent tearing */
	pinfo.lcd.v_back_porch = 11;
	pinfo.lcd.v_front_porch = 10;
	pinfo.lcd.v_pulse_width = 5;

	pinfo.mipi.mode = DSI_CMD_MODE;
	pinfo.mipi.dst_format = DSI_CMD_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
	pinfo.mipi.t_clk_post = 0x03;
	pinfo.mipi.t_clk_pre = 0x24;
	pinfo.mipi.stream = 0;	/* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_SW_TE;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.te_sel = 1; /* TE from vsycn gpio */
	pinfo.mipi.interleave_max = 1;
	pinfo.mipi.insert_dcs_cmd = TRUE;
	pinfo.mipi.wr_mem_continue = 0x3c;
	pinfo.mipi.wr_mem_start = 0x2c;
	pinfo.mipi.dsi_phy_db = &dsi_cmd_mode_phy_db;
	pinfo.mdp_dsi_calibration = mdp_dsi_calibration;
	pinfo.is_sw_resolve_esd_func = NULL;
	ret = mipi_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_QHD_PT);
	if (ret)
		pr_err("%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_otm_cmd_hd_pt_init);
