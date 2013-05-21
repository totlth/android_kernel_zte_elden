#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <mach/board.h>
#include <asm/mach-types.h>
#include <linux/input/atmel_qt602240.h>
#include <linux/jiffies.h>
#include <mach/msm_hsusb.h>
#include <mach/vreg.h>


struct atmel_config_data atmel_data = {
	.version = 0x16,
	.source = 1,
	.abs_x_min = 0,
	.abs_x_max = 319,
	.abs_y_min = 0,
	.abs_y_max = 479,
	.abs_pressure_min = 0,
	.abs_pressure_max = 255,
	.abs_width_min = 0,
	.abs_width_max = 15,
	//.gpio_irq = 55,
	//.power = atmel_platform_power,
	.config_T7[0] = 0x32,
	.config_T7[1] = 255,
	.config_T7[2] = 25,
	.config_T8[0] = 10,
	.config_T8[1] = 5,
	.config_T8[2] = 10,
	.config_T8[3] = 10,
  	.config_T8[4] = 10,
  	.config_T8[5] = 0,
  	.config_T8[6] = 0,
  	.config_T8[7] = 0,
  	.config_T8[8] = 15,
  	.config_T8[9] = 0xc0,
	
	.config_T9[0] = 0x8b,//0x8f
	.config_T9[1] = 0,
	.config_T9[2] = 2,
	.config_T9[3] = 14,//hwrev>5
	.config_T9[4] = 9,//hwrev>5
	.config_T9[5] = 0,//AKSCFG
	.config_T9[6] = 16,
	.config_T9[7] = 45,
	.config_T9[8] = 3, //2 touch detect integration 05-19
	//.config_T9[9] = 6,//∫·∆¡
	.config_T9[9] = 1,	// ˙∆¡
	.config_T9[10] = 10,
	.config_T9[11] = 2, //3 05-19
	.config_T9[12] = 2, //1 05-19
	.config_T9[13] = 79,//5
	.config_T9[14] = 5,
	.config_T9[15] = 10,
	.config_T9[16] = 30, //20 05-19
	.config_T9[17] = 30,
	.config_T9[18] = 0x0d,
	.config_T9[19] = 2,
	.config_T9[20] = 0x3f,
	.config_T9[21] = 1,
	//.config_T9[18] = 0x40,//320 low 16bits
	//.config_T9[19] = 0x01,// 320 high 16bits
	//.config_T9[20] = 0xdf,
	//.config_T9[21] = 0x01,
	//.config_T9[18] = 223,//31,
	//.config_T9[19] = 1,//3,
	//.config_T9[20] = 31,//223,
	//.config_T9[21] = 3,//1,  
	.config_T9[22] = 12,
	.config_T9[23] = 10,
	.config_T9[24] = 28,
	.config_T9[25] = 28,
	.config_T9[26] = 208,
	.config_T9[27] = 64,
	.config_T9[28] = 208,
	.config_T9[29] = 85,

	.config_T9[30] = 15,//10
	.config_T9[31] = 10,//10

	.config_T15[0] = 0,
	.config_T15[1] = 0,
	.config_T15[2] = 0,
	.config_T15[3] = 0,//4
	.config_T15[4] = 0,
	.config_T15[5] = 0,
	.config_T15[6] = 0,
	.config_T15[7] = 0,//25
	.config_T15[8] = 0,
	.config_T18[0] = 0,
	.config_T18[1] = 0,
	.config_T19[0] = 0,
	.config_T19[1] = 0,
	.config_T19[2] = 0,
	.config_T19[3] = 0,
	.config_T19[4] = 0,
	.config_T19[5] = 0,
	.config_T19[6] = 0,
	.config_T19[7] = 0,
	.config_T19[8] = 0,
	.config_T19[9] = 0,
	.config_T19[10] = 0,
	.config_T19[11] = 0,
  .config_T20[0] = 0x13,
  .config_T20[1] = 1,
  .config_T20[2] = 1,
  .config_T20[3] = 1,
  .config_T20[4] = 1,
  .config_T20[5] = 10,
  .config_T20[6] = 0,
  .config_T20[7] = 25,
  .config_T20[8] = 40,
  .config_T20[9] = 4,
  .config_T20[10] = 15,
  .config_T20[11] = 0,
	.config_T22[0] = 0x0d,
	.config_T22[1] = 0,
	.config_T22[2] = 0,
	.config_T22[3] = 0x19,//??
	.config_T22[4] = 0x0,//??
	.config_T22[5] = 0xE7,//??
	.config_T22[6] = 0xFF,//??
	.config_T22[7] = 8,
	.config_T22[8] = 25,
	.config_T22[9] = 0,
	.config_T22[10] = 1,
	.config_T22[11] = 19,
	.config_T22[12] = 0,
	.config_T22[13] = 25,
	.config_T22[14] = 45,
	.config_T22[15] = 0,
	.config_T22[16] = 8,
	.config_T23[0] = 0,
	.config_T23[1] = 0,
	.config_T23[2] = 0,
	.config_T23[3] = 0,
	.config_T23[4] = 0,
	.config_T23[5] = 0,
	.config_T23[6] = 0,
	.config_T23[7] = 0,
	.config_T23[8] = 0,
	.config_T23[9] = 0,
	.config_T23[10] = 0,
	.config_T23[11] = 0,
	.config_T23[12] = 0,
	.config_T24[0] = 0,
	.config_T24[1] = 0,
	.config_T24[2] = 0,
	.config_T24[3] = 0,
	.config_T24[4] = 0,
	.config_T24[5] = 0,
	.config_T24[6] = 0,
	.config_T24[7] = 0,
	.config_T24[8] = 0,
	.config_T24[9] = 0,
	.config_T24[10] = 0,
	.config_T24[11] = 0,
	.config_T24[12] = 0,
	.config_T24[13] = 0,
	.config_T24[14] = 0,
	.config_T24[15] = 0,
	.config_T24[16] = 0,
	.config_T24[17] = 0,
	.config_T24[18] = 0,
	.config_T25[0] = 0,
	.config_T25[1] = 0,
	.config_T25[2] = 0,
	.config_T25[3] = 0,
	.config_T25[4] = 0,
	.config_T25[5] = 0,
	.config_T25[5] = 0,
	.config_T25[6] = 0,
	.config_T25[7] = 0,
	.config_T25[8] = 0,
	.config_T25[9] = 0,
	.config_T27[0] = 0,
	.config_T27[1] = 1,
	.config_T27[2] = 0,
	.config_T27[3] = 0,
	.config_T27[4] = 0,
	.config_T27[5] = 0,
	.config_T27[6] = 0,
	.config_T28[0] = 0,
	.config_T28[1] = 0,
	.config_T28[2] = 1,
	.config_T28[3] = 16,
	.config_T28[4] = 16,//32,
	.config_T28[5] = 0,//0x0a
/*	.config_T59[0] = 7,
	.config_T59[1] = 36,  
	.config_T59[2] = 255,
	.config_T59[3] = 10, 
	.config_T59[4] = 1,
	.config_T59[5] = 15,    	
//#ifdef CONFIG_ATMEL_TS_USB_NOTIFY
	.config_T9_charge[0] = 70,//10	 //t9[7]
	.config_T9_charge[1] = 0,//10	  //t9[31]
	.config_T9_charge[2] = 10,//10	//t9[11]
	.config_T9_charge[3] = 10,//10	//t9[12]
	.config_T28_charge[0] = 32,//0x0a
	.config_T28_charge[1] = 32,//0x0a	
//#endif
	.object_crc[0] = 117,
	.object_crc[1] = 8,
	.object_crc[2] = 119,
.cable_config[0] = config_T9[7],
.cable_config[1] = config_T22[8],
.cable_config[2] = config_T28[3],
.cable_config[3] = config_T28[4],*/
	.cable_config[0] = 30,
	.cable_config[1] = 20,
	.cable_config[2] = 4,
	.cable_config[3] = 8,
	.GCAF_level[0] = 4,
	.GCAF_level[1] = 16,
	.GCAF_level[2] = 0,
	.GCAF_level[3] = 0,
	.filter_level[0] = 100,
	.filter_level[1] = 100,
	.filter_level[2] = 100,
	.filter_level[3] = 100,
	.display_width = 1024,  /* display width in pixel */
	.display_height = 1024, /* display height in pixel */
};
