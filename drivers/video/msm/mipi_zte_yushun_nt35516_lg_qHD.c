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

#include <mach/irqs.h>
#if( defined(CONFIG_MACH_DANA) || defined(CONFIG_MACH_JARVIS))
#define LCD_ID_PIN  3
#else
#define LCD_ID_PIN   90   //T82
#endif
extern u32 LcdPanleID ;


/* Macros assume PMIC GPIOs and MPPs start at 1 */
#define PM8921_GPIO_BASE		NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8921_GPIO_BASE)
#define PM8921_MPP_BASE			(PM8921_GPIO_BASE + PM8921_NR_GPIOS)
#define PM8921_MPP_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8921_MPP_BASE)
#define PM8921_IRQ_BASE			(NR_MSM_IRQS + NR_GPIO_IRQS)

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/* pan 540*960, RGB888, 2 Lane 60 fps video mode */
       /* regulator */
	{0x03, 0x0a, 0x04, 0x00, 0x20},
	/* timing */
	{0x66, 0x26, 0x16, 0x00, 0x19, 0x8e, 0x1e, 0x8c,
	0x19, 0x03, 0x04, 0xa0},
	/* phy ctrl */
	{0x5f, 0x00, 0x00, 0x10},
	/* strength */
	{0xff, 0x00, 0x06, 0x00},
	/* pll control */
	{0x0, 0x61, 0x1, 0x1a, 0x00, 0x50, 0x48, 0x63,
	0x30, 0x07, 0x03,//0x41, 0x0f, 0x01,
	0x00, 0x14, 0x03, 0x00, 0x02, 0x00, 0x20, 0x00, 0x01 },
		
	#if 0//back
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
	#endif
};



static int bl_lpm;
static struct mipi_dsi_panel_platform_data *mipi_toshiba_pdata;


static struct dsi_buf mipi_tx_buf;
static struct dsi_buf mipi_rx_buf;

static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

static struct dsi_cmd_desc display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};


/*******************  1 . truly lg lcd  **********end ***********************/

//lg lcd
static char para1[6]={0xFF,0xAA,0x55,0x25,0x01,0x01};
static char para2[31]={0xF2,0x00,0x00,0x4A,0x0A,0xA8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x01};
static char para3[8]={0xF3,0x02,0x03,0x07,0x45,0x88,0xD1,0x0D};
//Page 0 
static char para4[6]={0xF0,0x55,0xAA,0x52,0x08,0x00};
#if (defined (CONFIG_MACH_FROSTY)||defined(CONFIG_MACH_JARVIS))
static char para_0x36[4]={0x36,0xd4};//pan add for vertical flip
#endif

static char para5[4]={0xB1,0xFC,0x00,0x00};//only  set 0x36 = 0xd4 ;or  set 0x36 = 0x00 ,set  0xb101=0x06, 
static char para6[5]={0xB8,0x01,0x02,0x02,0x02};//02
static char para7[7]={0xC9,0x63,0x06,0x0D,0x1A,0x17,0x00};
//Page 1
static char para8[6]={0xF0,0x55,0xAA,0x52,0x08,0x01};
static char para9[4]={0xB0,0x05,0x05,0x05};
static char para10[4]={0xB1,0x05,0x05,0x05};
static char para11[4]={0xB2,0x01,0x01,0x01};
static char para12[4]={0xB3,0x0E,0x0E,0x0E};
static char para13[4]={0xB4,0x0a,0x0a,0x0a};
static char para14[4]={0xB6,0x44,0x44,0x44};
static char para15[4]={0xB7,0x34,0x34,0x34};
static char para16[4]={0xB8,0x10,0x10,0x10};
static char para17[4]={0xB9,0x26,0x26,0x26};
static char para18[4]={0xBA,0x24,0x24,0x24};
static char para19[4]={0xBC,0x00,0xC8,0x00};
static char para20[4]={0xBD,0x00,0xC8,0x00};
static char para21[2]={0xBE,0x7b}; //92
static char para22[3]={0xC0,0x04,0x00};
static char para23[2]={0xCA,0x00};

static char para24[5]={  0xD0, 0x0F, 0x0F, 0x10,0x10  };                                                                                    
                                                                                                                      
static char para25[17]={  0xD1, 0x00,0x70,0x00,0x83,0x00,0xA0,0x00,0xB9,0x00,0xCC,0x00,0xED,0x01,0x0C,0x01,0x3C  };                         
static char para26[17]={  0xD2, 0x01,0x62,0x01,0x9F,0x01,0xCE,0x02,0x14,0x02,0x50,0x02,0x51,0x02,0x88,0x02,0xC6  };                          
static char para27[17]={  0xD3, 0x02,0xED,0x03,0x20,0x03,0x41,0x03,0x67,0x03,0x80,0x03,0x9F,0x03,0xAF,0x03,0xEE  };                          
static char para28[5]={  0xD4, 0x03,0xFF,0x03,0xFF       };                                                                                 
                                                                                                                      
static char para29[17]={  0xD5, 0x00,0x70,0x00,0x83,0x00,0xA0,0x00,0xB9,0x00,0xCC,0x00,0xED,0x01,0x0C,0x01,0x3C  };                           
static char para30[17]={  0xD6, 0x01,0x62,0x01,0x9F,0x01,0xCE,0x02,0x14,0x02,0x50,0x02,0x51,0x02,0x88,0x02,0xC6  };                           
static char para31[17]={  0xD7, 0x02,0xED,0x03,0x20,0x03,0x41,0x03,0x67,0x03,0x80,0x03,0x9F,0x03,0xAF,0x03,0xEE };                            
static char para32[5]={  0xD8, 0x03,0xFF,0x03,0xFF  };                                                                                       
                                                                                                                      
static char para33[17]={  0xD9, 0x00,0x70,0x00,0x83,0x00,0xA0,0x00,0xB9,0x00,0xCC,0x00,0xED,0x01,0x0C,0x01,0x3C   };                         
static char para34[17]={  0xDD, 0x01,0x62,0x01,0x9F,0x01,0xCE,0x02,0x14,0x02,0x50,0x02,0x51,0x02,0x88,0x02,0xC6 };                           
static char para35[17]={  0xDE, 0x02,0xED,0x03,0x20,0x03,0x41,0x03,0x67,0x03,0x80,0x03,0x9F,0x03,0xAF,0x03,0xEE };                           
static char para36[5]={  0xDF, 0x03,0xFF,0x03,0xFF     };                                                                                   
                                                                                                                      
static char para37[17]={  0xE0, 0x00,0x70,0x00,0x83,0x00,0xA0,0x00,0xB9,0x00,0xCC,0x00,0xED,0x01,0x0C,0x01,0x3C  };                         
static char para38[17]={  0xE1, 0x01,0x62,0x01,0x9F,0x01,0xCE,0x02,0x14,0x02,0x50,0x02,0x51,0x02,0x88,0x02,0xC6  };                         
static char para39[17]={  0xE2, 0x02,0xED,0x03,0x20,0x03,0x41,0x03,0x67,0x03,0x80,0x03,0x9F,0x03,0xAF,0x03,0xEE  };                         
static char para40[5]={  0xE3, 0x03,0xFF,0x03,0xFF      };                                                                                 
                                                                                                                      
static char para41[17]={  0xE4, 0x00,0x70,0x00,0x83,0x00,0xA0,0x00,0xB9,0x00,0xCC,0x00,0xED,0x01,0x0C,0x01,0x3C  };                           
static char para42[17]={  0xE5, 0x01,0x62,0x01,0x9F,0x01,0xCE,0x02,0x14,0x02,0x50,0x02,0x51,0x02,0x88,0x02,0xC6  };                           
static char para43[17]={  0xE6, 0x02,0xED,0x03,0x20,0x03,0x41,0x03,0x67,0x03,0x80,0x03,0x9F,0x03,0xAF,0x03,0xEE  };                           
static char para44[5]={  0xE7, 0x03,0xFF,0x03,0xFF      };                                                       

static char para45[17]={  0xE8, 0x00,0x70,0x00,0x83,0x00,0xA0,0x00,0xB9,0x00,0xCC,0x00,0xED,0x01,0x0C,0x01,0x3C };
static char para46[17]={  0xE9, 0x01,0x62,0x01,0x9F,0x01,0xCE,0x02,0x14,0x02,0x50,0x02,0x51,0x02,0x88,0x02,0xC6 };
static char para47[17]={  0xEA, 0x02,0xED,0x03,0x20,0x03,0x41,0x03,0x67,0x03,0x80,0x03,0x9F,0x03,0xAF,0x03,0xEE };
static char para48[5]={  0xEB, 0x03,0xFF,0x03,0xFF    };   

static char para49[2]={0x2C,0x00};
static char para50[2]={0x13,0x00};

static struct dsi_cmd_desc truly_display_on_cmds[] = {

		  //for lg
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para1), para1},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para2), para2},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para3), para3},		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para4), para4},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para5), para5},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para6), para6},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para7), para7},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para8), para8},
		#if (defined (CONFIG_MACH_FROSTY)||defined(CONFIG_MACH_JARVIS))
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para_0x36), para_0x36},//pan add for vertical flip
		#endif
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para9), para9},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para10), para10},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para11), para11},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para12), para12},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para13), para13},		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para14), para14},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para15), para15},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para16), para16},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para17), para17},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para18), para18},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para19), para19},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para20), para20},
		{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(para21), para21},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para22), para22},
		{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(para23), para23},		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para24), para24},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para25), para25},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para26), para26},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para27), para27},	
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para28), para28},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para29), para29},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para30), para30},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para31), para31},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para32), para32},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para33), para33},		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para34), para34},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para35), para35},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para36), para36},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para37), para37},		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para38), para38},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para39), para39},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para40), para40},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para41), para41},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para42), para42},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para43), para43},		
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para44), para44},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para45), para45},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para46), para46},
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para47), para47},			
		{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(para48), para48},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(para49), para49},			
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(para50), para50},

		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
		{DTYPE_DCS_WRITE, 1, 0, 0, 20, sizeof(display_on), display_on}	

};
/******************* 1 .truly lg lcd  **********end ***********************/

#if 0
//< 2012/6/6-P826A10_add_new_lcd-lizhiye- < short commond here >
static char yunshun_para1[]={0xFF,0xAA,0x55,0x25,0x01};											
static char yunshun_para2[]={0xF2,0x00,0x00,0x4A,0x0A,0xA8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x01,0x51,0x00,0x01,0x00,0x01};	    
static char yunshun_para3[]={0xF3,0x02,0x03,0x07,0x45,0x88,0xD1,0x0D};										  
static char yunshun_para4[]={0xF0,0x55,0xAA,0x52,0x08,0x00};													        
static char yunshun_para5[]={0xB1,0xFC,0x00};														        
static char yunshun_para6[]={0xB8,0x01,0x02,0x02,0x02};													        
static char yunshun_para7[]={0xC9,0x63,0x06,0x0D,0x1A,0x17,0x00};												 
static char yunshun_para8[]={0xF0,0x55,0xAA,0x52,0x08,0x01};
static char yunshun_para_36[]={0x36,0xd4};
static char yunshun_para9[]={0xB0,0x05,0x05,0x05};														        
static char yunshun_para10[]={0xB1,0x05,0x05,0x05};														        
static char yunshun_para11[]={0xB2,0x01,0x01,0x01};													        
static char yunshun_para12[]={0xB3,0x0E,0x0E,0x0E};													        
static char yunshun_para13[]={0xB4,0x0A,0x0A,0x0A};													        
static char yunshun_para14[]={0xB6,0x44,0x44,0x44};													        
static char yunshun_para15[]={0xB7,0x34,0x34,0x34};													        
static char yunshun_para16[]={0xB8,0x20,0x20,0x20};															        
static char yunshun_para17[]={0xB9,0x26,0x26,0x26};															        
static char yunshun_para18[]={0xBA,0x24,0x24,0x24};														        
static char yunshun_para19[]={0xBC,0x00,0xC8,0x00};														        
static char yunshun_para20[]={0xBD,0x00,0xC8,0x00};														        
static char yunshun_para21[]={0xBE,0x92};																        
static char yunshun_para22[]={0xC0,0x04,0x00};															        
static char yunshun_para23[]={0xCA,0x00};														        
static char yunshun_para24[]={0xD0,0x0A,0x10,0x0D,0x0F};									
static char yunshun_para25[]={0xD1,0x00,0x70,0x00,0x92,0x00,0xD3,0x01,0x02,0x01,0x1C,0x01,0x38,0x01,0x4F,0x01,0x82};									        
static char yunshun_para26[]={0xD2,0x01,0xA6,0x01,0xDB,0x02,0x06,0x02,0x46,0x02,0x79,0x02,0x7A,0x02,0xAE,0x02,0xE7};
static char yunshun_para27[]={0xD3,0x03,0x08,0x03,0x36,0x03,0x51,0x03,0x77,0x03,0x8C,0x03,0xA8,0x03,0xB9,0x03,0xDD};  
static char yunshun_para28[]={0xD4,0x03,0xFD,0x03,0xFF};
static char yunshun_para29[]={0xD5,0x00,0x70,0x00,0x92,0x00,0xD3,0x01,0x02,0x01,0x1C,0x01,0x38,0x01,0x4F,0x01,0x82};									        
static char yunshun_para30[]={0xD6,0x01,0xA6,0x01,0xDB,0x02,0x06,0x02,0x46,0x02,0x79,0x02,0x7A,0x02,0xAE,0x02,0xE7};
static char yunshun_para31[]={0xD7,0x03,0x08,0x03,0x36,0x03,0x51,0x03,0x77,0x03,0x8C,0x03,0xA8,0x03,0xB9,0x03,0xDD};
static char yunshun_para32[]={0xD8,0x03,0xFD,0x03,0xFF};
static char yunshun_para33[]={0xD9,0x00,0x70,0x00,0x92,0x00,0xD3,0x01,0x02,0x01,0x1C,0x01,0x38,0x01,0x4F,0x01,0x82};											        
static char yunshun_para34[]={0xDD,0x01,0xA6,0x01,0xDB,0x02,0x06,0x02,0x46,0x02,0x79,0x02,0x7A,0x02,0xAE,0x02,0xE7};    
static char yunshun_para35[]={0xDE,0x03,0x08,0x03,0x36,0x03,0x51,0x03,0x77,0x03,0x8C,0x03,0xA8,0x03,0xB9,0x03,0xDD};   
static char yunshun_para36[]={0xDF,0x03,0xFD,0x03,0xFF};												
static char yunshun_para37[]={0xE0,0x00,0x70,0x00,0x92,0x00,0xD3,0x01,0x02,0x01,0x1C,0x01,0x38,0x01,0x4F,0x01,0x82};											        
static char yunshun_para38[]={0xE1,0x01,0xA6,0x01,0xDB,0x02,0x06,0x02,0x46,0x02,0x79,0x02,0x7A,0x02,0xAE,0x02,0xE7};  
static char yunshun_para39[]={0xE2,0x03,0x08,0x03,0x36,0x03,0x51,0x03,0x77,0x03,0x8C,0x03,0xA8,0x03,0xB9,0x03,0xDD}; 
static char yunshun_para40[]={0xE3,0x03,0xFD,0x03,0xFF};  
static char yunshun_para41[]={0xE4,0x00,0x70,0x00,0x92,0x00,0xD3,0x01,0x02,0x01,0x1C,0x01,0x38,0x01,0x4F,0x01,0x82};												        
static char yunshun_para42[]={0xE5,0x01,0xA6,0x01,0xDB,0x02,0x06,0x02,0x46,0x02,0x79,0x02,0x7A,0x02,0xAE,0x02,0xE7};     
static char yunshun_para43[]={0xE6,0x03,0x08,0x03,0x36,0x03,0x51,0x03,0x77,0x03,0x8C,0x03,0xA8,0x03,0xB9,0x03,0xDD};   
static char yunshun_para44[]={0xE7,0x03,0xFD,0x03,0xFF};												        
static char yunshun_para45[]={0xE8,0x00,0x70,0x00,0x92,0x00,0xD3,0x01,0x02,0x01,0x1C,0x01,0x38,0x01,0x4F,0x01,0x82};												        
static char yunshun_para46[]={0xE9,0x01,0xA6,0x01,0xDB,0x02,0x06,0x02,0x46,0x02,0x79,0x02,0x7A,0x02,0xAE,0x02,0xE7};      
static char yunshun_para47[]={0xEA,0x03,0x08,0x03,0x36,0x03,0x51,0x03,0x77,0x03,0x8C,0x03,0xA8,0x03,0xB9,0x03,0xDD};  
static char yunshun_para48[]={0xEB,0x03,0xFD,0x03,0xFF};																								        
static char yunshun_para49[]={0X11,0x00};																		        
static char yunshun_para50[]={0X3A,0X77};																									        
static char yunshun_para51[]={0X13,0x00};																																			        
static char yunshun_para52[]={0X29,0x00};																																			        

static struct dsi_cmd_desc yushun_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para1), yunshun_para1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para2), yunshun_para2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para3), yunshun_para3},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para4), yunshun_para4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para5), yunshun_para5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para6), yunshun_para6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para7), yunshun_para7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para8), yunshun_para8},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(yunshun_para_36), yunshun_para_36},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para9), yunshun_para9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para10), yunshun_para10},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para11), yunshun_para11},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para12), yunshun_para12},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para13), yunshun_para13},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para14), yunshun_para14},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para15), yunshun_para15},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para16), yunshun_para16},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para17), yunshun_para17},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para18), yunshun_para18},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para19), yunshun_para19},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para20), yunshun_para20},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(yunshun_para21), yunshun_para21},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para22), yunshun_para22},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(yunshun_para23), yunshun_para23},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para24), yunshun_para24},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para25), yunshun_para25},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para26), yunshun_para26},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para27), yunshun_para27},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para28), yunshun_para28},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para29), yunshun_para29},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para30), yunshun_para30},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para31), yunshun_para31},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para32), yunshun_para32},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para33), yunshun_para33},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para34), yunshun_para34},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para35), yunshun_para35},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para36), yunshun_para36},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para37), yunshun_para37},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para38), yunshun_para38},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para39), yunshun_para39},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para40), yunshun_para40},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para41), yunshun_para41},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para42), yunshun_para42},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para43), yunshun_para43},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para44), yunshun_para44},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para45), yunshun_para45},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para46), yunshun_para46},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para47), yunshun_para47},			
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(yunshun_para48), yunshun_para48},
	{DTYPE_DCS_WRITE, 1, 0, 0, 200, sizeof(yunshun_para49), yunshun_para49},			
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(yunshun_para50), yunshun_para50},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(yunshun_para51), yunshun_para51},			
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(yunshun_para52), yunshun_para52},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 20, sizeof(display_on), display_on}	
};
//>2012/6/6-P826A10_add_new_lcd-lizhiye
#endif

/******************* 2. lead auo lcd  **********start ***********************/
static char auo_para0_1[5]={0xFF,0xAA,0x55,0x25,0x01};
static char auo_para0_2[5]={0xF3,0x02,0x03,0x07,0x45};
// Select CMD2, Page 0 
static char auo_para1[6]={0xF0,0x55,0xAA,0x52,0x08,0x00};
// Video mode Enable
static char auo_para1_1[2]={0xB1,0xFC};
// Source EQ
static char auo_para2[5]={0xB8,0x01,0x02,0x02,0x02};
// Z Inversion
static char auo_para3[4]={0xBC,0x05,0x05,0x05};
// Vivid Color
static char auo_para3_1[2]={0xD8,0x40};
static char auo_para3_2[17]={0xD6,0x00,0x03,0x05,0x08,0x0A,0x0D,0x0F,0x12,0x14,0x17,0x1A,0x1C,0x1F,0x21,0x24,0x26};
static char auo_para3_3[9]={0xD7,0x29,0x2C,0x2E,0x31,0x33,0x36,0x38,0x3D};
#if (defined (CONFIG_MACH_FROSTY)||defined(CONFIG_MACH_JARVIS))
static char auo_para_0x36[2]={0x36,0xd4};//pan add for vertical flip
#endif

// Select CMD2, Page 1
static char auo_para4[6]={0xF0,0x55,0xAA,0x52,0x08,0x01};
// AVDD: 6.0V
static char auo_para5[4]={0xB0,0x05,0x05,0x05};
// AVEE: -6.0V
static char auo_para6[4]={0xB1,0x05,0x05,0x05};
// VGH: 15V
static char auo_para6_1[4]={0xB3,0x10,0x10,0x10};
// VGL: -12V
static char auo_para6_2[4]={0xB4,0x0A,0x0A,0x0A};
// AVDD: 2.5x VPNL
static char auo_para7[4]={0xB6,0x44,0x44,0x44};
// AVEE: -2.5x VPNL
static char auo_para8[4]={0xB7,0x34,0x34,0x34};
// VGH: 2AVDD-AVEE
static char auo_para81[4]={0xB9,0x34,0x34,0x34};
// VGLX: AVEE - AVDD + VCL
static char auo_para9[4]={0xBA,0x34,0x34,0x34};
// VGMP: 5.0V, VGSP=0V
static char auo_para10[4]={0xBC,0x00,0xA0,0x00};
// VGMN: 5.0V, VGSN=-0V
static char auo_para11[4]={0xBD,0x00,0xA0,0x00};
// VCOM
static char auo_para12[2]={0xBE,0x4F};
//Vivid color Enable
static char auo_para12_1[2]={0x4C,0x11};

// Gamma Code

// Positive Red Gamma
static char auo_para13[17]={0xD1,0x00,0x70,0x00,0x7B,0x00,0x8E,0x00,0x9F,0x00,0xAD,0x00,0xC8,0x00,0xE0,0x01,0x04};
static char auo_para14[17]={0xD2,0x01,0x22,0x01,0x51,0x01,0x76,0x01,0xAF,0x01,0xDC,0x01,0xDD,0x02,0x05,0x02,0x2E};
static char auo_para15[17]={0xD3,0x02,0x43,0x02,0x64,0x02,0x7A,0x02,0xA6,0x02,0xC9,0x03,0x1B,0x03,0x51,0x03,0xFF};
static char auo_para16[5]={0xD4,0x03,0xFF,0x03,0xFF};

// Positive Green Gamma
static char auo_para17[17]={0xD5,0x00,0x70,0x00,0x7B,0x00,0x8E,0x00,0x9F,0x00,0xAD,0x00,0xC8,0x00,0xE0,0x01,0x04};
static char auo_para18[17]={0xD6,0x01,0x22,0x01,0x51,0x01,0x76,0x01,0xAF,0x01,0xDC,0x01,0xDD,0x02,0x05,0x02,0x2E};
static char auo_para19[17]={0xD7,0x02,0x43,0x02,0x64,0x02,0x7A,0x02,0xA6,0x02,0xC9,0x03,0x1B,0x03,0x51,0x03,0xFF};
static char auo_para20[5]={0xD8,0x03,0xFF,0x03,0xFF};

// Positive Blue Gamma
static char auo_para21[17]={0xD9,0x00,0x70,0x00,0x7B,0x00,0x8E,0x00,0x9F,0x00,0xAD,0x00,0xC8,0x00,0xE0,0x01,0x04};
static char auo_para22[17]={0xDD,0x01,0x22,0x01,0x51,0x01,0x76,0x01,0xAF,0x01,0xDC,0x01,0xDD,0x02,0x05,0x02,0x2E};
static char auo_para23[17]={0xDE,0x02,0x43,0x02,0x64,0x02,0x7A,0x02,0xA6,0x02,0xC9,0x03,0x1B,0x03,0x51,0x03,0xFF};
static char auo_para24[5]={0xDF,0x03,0xFF,0x03,0xFF};

// Negative Red Gamma
static char auo_para25[17]={0xE0,0x00,0x70,0x00,0x7B,0x00,0x8E,0x00,0x9F,0x00,0xAD,0x00,0xC8,0x00,0xE0,0x01,0x04};
static char auo_para26[17]={0xE1,0x01,0x22,0x01,0x51,0x01,0x76,0x01,0xAF,0x01,0xDC,0x01,0xDD,0x02,0x05,0x02,0x2E};
static char auo_para27[17]={0xE2,0x02,0x43,0x02,0x64,0x02,0x7A,0x02,0xA6,0x02,0xC9,0x03,0x1B,0x03,0x51,0x03,0xFF};
static char auo_para28[5]={0xE3,0x03,0xFF,0x03,0xFF};

// Negative Green Gamma
static char auo_para29[17]={0xE4,0x00,0x70,0x00,0x7B,0x00,0x8E,0x00,0x9F,0x00,0xAD,0x00,0xC8,0x00,0xE0,0x01,0x04};
static char auo_para30[17]={0xE5,0x01,0x22,0x01,0x51,0x01,0x76,0x01,0xAF,0x01,0xDC,0x01,0xDD,0x02,0x05,0x02,0x2E};
static char auo_para31[17]={0xE6,0x02,0x43,0x02,0x64,0x02,0x7A,0x02,0xA6,0x02,0xC9,0x03,0x1B,0x03,0x51,0x03,0xFF};
static char auo_para32[5]={0xE7,0x03,0xFF,0x03,0xFF};

// Negative Blue Gamma
static char auo_para33[17]={0xE8,0x00,0x70,0x00,0x7B,0x00,0x8E,0x00,0x9F,0x00,0xAD,0x00,0xC8,0x00,0xE0,0x01,0x04};
static char auo_para34[17]={0xE9,0x01,0x22,0x01,0x51,0x01,0x76,0x01,0xAF,0x01,0xDC,0x01,0xDD,0x02,0x05,0x02,0x2E};
static char auo_para35[17]={0xEA,0x02,0x43,0x02,0x64,0x02,0x7A,0x02,0xA6,0x02,0xC9,0x03,0x1B,0x03,0x51,0x03,0xFF};
static char auo_para36[5]={0xEB,0x03,0xFF,0x03,0xFF};
// TE On
static char auo_para37[2]={0x35,0x00};


static struct dsi_cmd_desc auo_display_on_cmds[] = {

	   //for auo
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para0_1), auo_para0_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para0_2), auo_para0_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para1), auo_para1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para1_1), auo_para1_1},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para2), auo_para2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para3), auo_para3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para3_1), auo_para3_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para3_2), auo_para3_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para3_3), auo_para3_3},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para4), auo_para4},
#if (defined (CONFIG_MACH_FROSTY)||defined(CONFIG_MACH_JARVIS))
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para_0x36), auo_para_0x36},	
#endif
	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para5), auo_para5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para6), auo_para6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para6_1), auo_para6_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para6_2), auo_para6_2},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para7), auo_para7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para8), auo_para8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para81), auo_para81},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para9), auo_para9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para10), auo_para10},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para11), auo_para11},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(auo_para12), auo_para12},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(auo_para12_1), auo_para12_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para13), auo_para13},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para14), auo_para14},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para15), auo_para15},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para16), auo_para16},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para17), auo_para17},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para18), auo_para18},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para19), auo_para19},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para20), auo_para20},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para21), auo_para21},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para22), auo_para22},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para23), auo_para23},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para24), auo_para24},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para25), auo_para25},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para26), auo_para26},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para27), auo_para27},	
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para28), auo_para28},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para29), auo_para29},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para30), auo_para30},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para31), auo_para31},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para32), auo_para32},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para33), auo_para33},		
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para34), auo_para34},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para35), auo_para35},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(auo_para36), auo_para36},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(auo_para37), auo_para37},
	
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 20, sizeof(display_on), display_on}

};

/******************* 2. lead auo lcd  *********end***********************/



/******************* 3. sharp  lcd  **********start ***********************/

static char sharp_para1[6]={0xF0,0x55,0xAA,0x52,0x08,0x01};
static char sharp_para2[2]={0x36,0x50};

static struct dsi_cmd_desc sharp_display_on_cmds[] = {

	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(sharp_para1), sharp_para1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(sharp_para2), sharp_para2},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 20, sizeof(display_on), display_on}
};


/******************* 3. sharp  lcd  *********end***********************/

int get_io_value(uint32 pin)
{
	int rc , id_pin = pin;
	rc = gpio_tlmm_config(GPIO_CFG(id_pin, 0,
    				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
    				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    	  if (rc) {
    			printk(KERN_ERR"%s: Could not configure nfc gpio %d\n",
    					__func__,id_pin);
    			 return -EIO;
    		   }
    
    	 rc = gpio_request(id_pin, "id_pin");
    	 if (rc) {
    			printk(KERN_ERR"%s: unable to request nfc gpio %d (%d)\n",
    					__func__, id_pin, rc);
    			 return -EIO;
    		    }

	rc = gpio_get_value(id_pin);
	printk("\n lcd id pin value = %d" ,rc );
	return rc;
		 
}

void get_panel_id(void)
{
	// if (get_io_value(LCD_ID_PIN)==1)
	// 	LcdPanleID = LCD_PANEL_4P3_NT35516_LG_TRULY;
	// else
	// 	LcdPanleID = LCD_PANEL_4P3_NT35516_AUO_LEAD;
	
	if (get_io_value(LCD_ID_PIN)==0)
		LcdPanleID = LCD_PANEL_4P3_NT35516_AUO_LEAD;
	else
	{
	#if 0//def CONFIG_MACH_JARVIS
		LcdPanleID = LCD_PANEL_4P3_NT35516_SHARP_SHARP;
	#else
		LcdPanleID = LCD_PANEL_4P3_NT35516_LG_TRULY;
	#endif
	}

}
static void lcd_panle_reset(void)
{
	int gpio_rst;
	gpio_rst = PM8921_GPIO_PM_TO_SYS(43);
	gpio_set_value_cansleep(gpio_rst, 1); /* disp enable */
	msleep(10);
	gpio_set_value_cansleep(gpio_rst, 0); /* disp enable */
	msleep(10);
	gpio_set_value_cansleep(gpio_rst, 1); /* disp enable */
	msleep(120);  
}

static int firsttimeboot=1;


static int mipi_toshiba_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
		
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	if( firsttimeboot== 1)
	{
		firsttimeboot = 0;
		return 0;
	}
	
	lcd_panle_reset();
	
	switch(LcdPanleID)
	{
		case (uint32)LCD_PANEL_4P3_NT35516_AUO_LEAD:
			mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, auo_display_on_cmds,
			ARRAY_SIZE(auo_display_on_cmds));
			printk("\n lead panel initialization ok!");
		break;
		case (uint32)LCD_PANEL_4P3_NT35516_LG_TRULY:
			mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, truly_display_on_cmds,
			ARRAY_SIZE(truly_display_on_cmds));
			printk("\n truly and yushun panel initialization ok!");
		break;
		case (uint32)LCD_PANEL_4P3_NT35516_SHARP_SHARP:
			mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, sharp_display_on_cmds,
			ARRAY_SIZE(sharp_display_on_cmds));
			printk("\n sharp initialization ok!");
			break;
		default:
			printk("\n panel initialization error !");
			get_panel_id();
			break;

	}	

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
		

	mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, display_off_cmds,
			ARRAY_SIZE(display_off_cmds));
	
	lcd_panle_reset();//for low power consumption
	
	return 0;
}

static bool onewiremode = false;
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
#if ((defined CONFIG_MACH_FROSTY)|| defined(CONFIG_MACH_DANA))
    if(current_lel > 27)
    {
        current_lel = 27;
    }
#else
    if(current_lel > 28)
    {
        current_lel = 28;
    }
#endif

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
		bl_lpm = 12; //mipi_toshiba_pdata->gpio[0];  
	//gpio_request(bl_lpm,"backlight");
	if( LcdPanleID==LCD_PANEL_NOPANEL)
	{
		printk("\n could not get panel id from boot !");
		get_panel_id();
	}
	
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
	mipi_dsi_buf_alloc(&mipi_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&mipi_rx_buf, DSI_BUF_SIZE);

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
#else//pan change proch for 60fps
		pinfo.lcdc.h_back_porch = 80;//100;
		pinfo.lcdc.h_front_porch = 80;//100;
		pinfo.lcdc.h_pulse_width = 8;
		pinfo.lcdc.v_back_porch =11;// 8;//10;
		pinfo.lcdc.v_front_porch = 5;//8;//5;
		pinfo.lcdc.v_pulse_width = 8;
#endif

		pinfo.lcdc.border_clr = 0;	/* blk */
		pinfo.lcdc.underflow_clr = 0;//0xff;	/* blue */
		pinfo.lcdc.hsync_skew = 0;
#if ((defined CONFIG_MACH_FROSTY)|| defined(CONFIG_MACH_DANA))
		pinfo.bl_max = 27;
#else
		pinfo.bl_max = 28;
#endif
		pinfo.bl_min = 1;
		pinfo.fb_num = 2;
		pinfo.clk_rate =499000000;//pan   384000000;
	
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

