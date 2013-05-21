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


#include "mipi_otm_hd.h"



extern u32 LcdPanleID;
extern struct workqueue_struct *lcd_esd_queue;
extern struct work_struct lcd_esd_work;
extern struct timer_list lcd_esd_timer;
extern struct workqueue_struct *lcd_esd_queue;
extern char timer_deleted;

struct msm_fb_data_type *local_mfd=NULL;
static int bl_lpm;
static int gpio_rst;
static struct mipi_dsi_panel_platform_data *mipi_toshiba_pdata;
 struct dsi_buf mipi_tx_buf;
 struct dsi_buf mipi_rx_buf;
static int firsttimeboot = 1;
static bool onewiremode = true;

static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};

 void lcd_panle_reset(void)
{
	gpio_set_value_cansleep(gpio_rst, 1); 
	msleep(10);
	gpio_set_value_cansleep(gpio_rst, 0); 
	msleep(10);
	gpio_set_value_cansleep(gpio_rst, 1);
	msleep(40); 
}


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
		local_mfd=mfd;
		firsttimeboot = 0;
		printk("LCD, first time run!!!!\n");
		if(mfd->panel_info.is_sw_resolve_esd_func){
			printk("LCD use sw resolve esd\n");
			lcd_esd_queue = create_workqueue("lcd_esd");
			INIT_WORK(&lcd_esd_work, lcd_esd_work_func);
			init_timer(&lcd_esd_timer);
			lcd_esd_timer.function = lcd_esd_state_timer;
			lcd_esd_timer.data = (uint32)NULL;
			lcd_esd_timer.expires = jiffies+40*HZ;
			add_timer(&lcd_esd_timer);
		}
		if(LcdPanleID != LCD_PANEL_NOPANEL)
			return 0;
		else{
			LcdPanleID = mipi_get_icpanleid(mfd);
			lcd_init_prepare(LcdPanleID);
		}

	}
	
	lcd_panle_reset();
	mipi_set_tx_power_mode(1);
	printk("LCD: LCD_INIT_START\n");
	mipi_tx_init_to_lcd(mfd);

	mipi_set_tx_power_mode(0);
	if(mfd->panel_info.is_sw_resolve_esd_func)
		if(timer_deleted)
		{
			mod_timer(&lcd_esd_timer,  jiffies+HZ*1);
			timer_deleted=0;
		}

	return 0;
}

static int mipi_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	
	printk("\nLCD %s: \n", __func__);
	
	mfd = platform_get_drvdata(pdev);
	
	if(mfd->panel_info.is_sw_resolve_esd_func){
		del_timer(&lcd_esd_timer);
		timer_deleted=1;
	}
	
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

 void mipi_set_backlight(struct msm_fb_data_type *mfd)
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

 uint32 mipi_read_commic_reg(struct msm_fb_data_type *mfd,struct dsi_cmd_desc *para,uint32 len,int mode)
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

	if (mipi_toshiba_pdata != NULL){
		bl_lpm = mipi_toshiba_pdata->gpio[0];
		gpio_rst = mipi_toshiba_pdata->gpio[1];
	}
	lcd_init_prepare(LcdPanleID);
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
