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

#include "msm_sensor.h"
#include "msm.h"
#include "msm_ispif.h"
#define SENSOR_NAME "mt9m114"
#define PLATFORM_DRIVER_NAME "msm_camera_mt9m114"
#define mt9m114_obj mt9m114_##obj

/* Sysctl registers */
#define MT9M114_COMMAND_REGISTER                0x0080
#define MT9M114_COMMAND_REGISTER_APPLY_PATCH    (1 << 0)
#define MT9M114_COMMAND_REGISTER_SET_STATE      (1 << 1)
#define MT9M114_COMMAND_REGISTER_REFRESH        (1 << 2)
#define MT9M114_COMMAND_REGISTER_WAIT_FOR_EVENT (1 << 3)
#define MT9M114_COMMAND_REGISTER_OK             (1 << 15)

extern void msm_sensorinfo_set_front_sensor_id(uint16_t id);

DEFINE_MUTEX(mt9m114_mut);
static struct msm_sensor_ctrl_t mt9m114_s_ctrl;


static struct msm_camera_i2c_reg_conf mt9m114_720p_settings[] = {
	{0xdc00, 0x50, MSM_CAMERA_I2C_BYTE_DATA, MSM_CAMERA_I2C_CMD_WRITE},
	{MT9M114_COMMAND_REGISTER, MT9M114_COMMAND_REGISTER_SET_STATE,
		MSM_CAMERA_I2C_UNSET_WORD_MASK, MSM_CAMERA_I2C_CMD_POLL},
	{MT9M114_COMMAND_REGISTER, (MT9M114_COMMAND_REGISTER_OK |
		MT9M114_COMMAND_REGISTER_SET_STATE), MSM_CAMERA_I2C_WORD_DATA,
		MSM_CAMERA_I2C_CMD_WRITE},
	{MT9M114_COMMAND_REGISTER, MT9M114_COMMAND_REGISTER_SET_STATE,
		MSM_CAMERA_I2C_UNSET_WORD_MASK, MSM_CAMERA_I2C_CMD_POLL},
	{0xDC01, 0x52, MSM_CAMERA_I2C_BYTE_DATA, MSM_CAMERA_I2C_CMD_POLL},

	{0x098E, 0, MSM_CAMERA_I2C_BYTE_DATA},
	{0xC800, 0x007C,},/*y_addr_start = 124*/
	{0xC802, 0x0004,},/*x_addr_start = 4*/
	{0xC804, 0x0353,},/*y_addr_end = 851*/
	{0xC806, 0x050B,},/*x_addr_end = 1291*/
	{0xC808, 0x02DC,},/*pixclk = 48000000*/
	{0xC80A, 0x6C00,},/*pixclk = 48000000*/
	{0xC80C, 0x0001,},/*row_speed = 1*/
	{0xC80E, 0x00DB,},/*fine_integ_time_min = 219*/
	{0xC810, 0x05BD,},/*fine_integ_time_max = 1469*/
	{0xC812, 0x03E8,},/*frame_length_lines = 1000*/
	{0xC814, 0x0640,},/*line_length_pck = 1600*/
	{0xC816, 0x0060,},/*fine_correction = 96*/
	{0xC818, 0x02D3,},/*cpipe_last_row = 723*/
	{0xC826, 0x0020,},/*reg_0_data = 32*/
	{0xC834, 0x0003,},//0x0000/*sensor_control_read_mode = 0*/
	{0xC854, 0x0000,},/*crop_window_xoffset = 0*/
	{0xC856, 0x0000,},/*crop_window_yoffset = 0*/
	{0xC858, 0x0500,},/*crop_window_width = 1280*/
	{0xC85A, 0x02D0,},/*crop_window_height = 720*/
	{0xC85C, 0x03, MSM_CAMERA_I2C_BYTE_DATA},  /*crop_cropmode = 3*/
	{0xC868, 0x0500,},/*output_width = 1280*/
	{0xC86A, 0x02D0,},/*output_height = 720*/
	{0xC878, 0x00, MSM_CAMERA_I2C_BYTE_DATA},  /*aet_aemode = 0*/
	{0xC88C, 0x1E00,},/*aet_max_frame_rate = 7680*/
	{0xC88E, 0x1E00,},/*aet_min_frame_rate = 7680*/
	{0xC914, 0x0000,},/*stat_awb_window_xstart = 0*/
	{0xC916, 0x0000,},/*stat_awb_window_ystart = 0*/
	{0xC918, 0x04FF,},/*stat_awb_window_xend = 1279*/
	{0xC91A, 0x02CF,},/*stat_awb_window_yend = 719*/
	{0xC91C, 0x0000,},/*stat_ae_window_xstart = 0*/
	{0xC91E, 0x0000,},/*stat_ae_window_ystart = 0*/
	{0xC920, 0x00FF,},/*stat_ae_window_xend = 255*/
	{0xC922, 0x008F,},/*stat_ae_window_yend = 143*/
};

static struct msm_camera_i2c_reg_conf mt9m114_recommend_settings1[] = {
	{0x3C40, 0x003C,},
	{0x301A, 0x0200, MSM_CAMERA_I2C_SET_WORD_MASK},
	{0x098E, 0, MSM_CAMERA_I2C_BYTE_DATA},
	/*cam_sysctl_pll_enable = 1*/
	{0xC97E, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	/*cam_sysctl_pll_divider_m_n = 288*/
	{0xC980, 0x0120,},
	/*cam_sysctl_pll_divider_p = 1792*/
	{0xC982, 0x0700,},
	/*output_control = 32769*/

	{0xC800, 0x007C,},/*y_addr_start = 124*/
	{0xC802, 0x0004,},/*x_addr_start = 4*/
	{0xC804, 0x0353,},/*y_addr_end = 851*/
	{0xC806, 0x050B,},/*x_addr_end = 1291*/
	{0xC808, 0x02DC,},/*pixclk = 48000000*/
	{0xC80A, 0x6C00,},/*pixclk = 48000000*/
	{0xC80C, 0x0001,},/*row_speed = 1*/
	{0xC80E, 0x00DB,},/*fine_integ_time_min = 219*/
	{0xC810, 0x05BD,},/*fine_integ_time_max = 1469*/
	{0xC812, 0x03E8,},/*frame_length_lines = 1000*/
	{0xC814, 0x0640,},/*line_length_pck = 1600*/
	{0xC816, 0x0060,},/*fine_correction = 96*/
	{0xC818, 0x02D3,},/*cpipe_last_row = 723*/
	{0xC826, 0x0020,},/*reg_0_data = 32*/
	{0xC834, 0x0003,},//0x0000/*sensor_control_read_mode = 0*/
	{0xC854, 0x0000,},/*crop_window_xoffset = 0*/
	{0xC856, 0x0000,},/*crop_window_yoffset = 0*/
	{0xC858, 0x0500,},/*crop_window_width = 1280*/
	{0xC85A, 0x02D0,},/*crop_window_height = 720*/
	{0xC85C, 0x03, MSM_CAMERA_I2C_BYTE_DATA},  /*crop_cropmode = 3*/
	{0xC868, 0x0500,},/*output_width = 1280*/
	{0xC86A, 0x02D0,},/*output_height = 720*/
	{0xC878, 0x00, MSM_CAMERA_I2C_BYTE_DATA},  /*aet_aemode = 0*/
	{0xC88C, 0x1E00,},/*aet_max_frame_rate = 7680*/
	{0xC88E, 0x1E00,},/*aet_min_frame_rate = 7680*/
	{0xC914, 0x0000,},/*stat_awb_window_xstart = 0*/
	{0xC916, 0x0000,},/*stat_awb_window_ystart = 0*/
	{0xC918, 0x04FF,},/*stat_awb_window_xend = 1279*/
	{0xC91A, 0x02CF,},/*stat_awb_window_yend = 719*/
	{0xC91C, 0x0000,},/*stat_ae_window_xstart = 0*/
	{0xC91E, 0x0000,},/*stat_ae_window_ystart = 0*/
	{0xC920, 0x00FF,},/*stat_ae_window_xend = 255*/
	{0xC922, 0x008F,},/*stat_ae_window_yend = 143*/

	/*Sensor optimization*/
	{0x316A, 0x8270,},
	{0x316C, 0x8270,},
	{0x3ED0, 0x2305,},
	{0x3ED2, 0x77CF,},
	{0x316E, 0x8202,},
	{0x3180, 0x87FF,},
	{0x30D4, 0x6080,},
	{0xA802, 0x0008,},/*AE_TRACK_MODE*/
	{0x3E14, 0xFF39,},
	{0x31E0, 0x0001,}, //lowlux 0828

	//[patch 1204]for 720P
	{0x0982, 0x0001,}, 	// ACCESS_CTL_STAT
	{0x098A, 0x60BC,}, 	// PHYSICAL_ADDRESS_ACCESS
	{0xE0BC, 0xC0F1,},
	{0xE0BE, 0x082A,},
	{0xE0C0, 0x05A0,},
	{0xE0C2, 0xD800,},
	{0xE0C4, 0x71CF,},
	{0xE0C6, 0xFFFF,},
	{0xE0C8, 0xC344,},
	{0xE0CA, 0x77CF,},
	{0xE0CC, 0xFFFF,},
	{0xE0CE, 0xC7C0,},
	{0xE0D0, 0xB104,},
	{0xE0D2, 0x8F1F,},
	{0xE0D4, 0x75CF,},
	{0xE0D6, 0xFFFF,},
	{0xE0D8, 0xC84C,},
	{0xE0DA, 0x0811,},
	{0xE0DC, 0x005E,},
	{0xE0DE, 0x70CF,},
	{0xE0E0, 0x0000,},
	{0xE0E2, 0x500E,},
	{0xE0E4, 0x7840,},
	{0xE0E6, 0xF019,},
	{0xE0E8, 0x0CC6,},
	{0xE0EA, 0x0340,},
	{0xE0EC, 0x0E26,},
	{0xE0EE, 0x0340,},
	{0xE0F0, 0x95C2,},
	{0xE0F2, 0x0E21,},
	{0xE0F4, 0x101E,},
	{0xE0F6, 0x0E0D,},
	{0xE0F8, 0x119E,},
	{0xE0FA, 0x0D56,},
	{0xE0FC, 0x0340,},
	{0xE0FE, 0xF008,},
	{0xE100, 0x2650,},
	{0xE102, 0x1040,},
	{0xE104, 0x0AA2,},
	{0xE106, 0x0360,},
	{0xE108, 0xB502,},
	{0xE10A, 0xB5C2,},
	{0xE10C, 0x0B22,},
	{0xE10E, 0x0400,},
	{0xE110, 0x0CCE,},
	{0xE112, 0x0320,},
	{0xE114, 0xD800,},
	{0xE116, 0x70CF,},
	{0xE118, 0xFFFF,},
	{0xE11A, 0xC5D4,},
	{0xE11C, 0x902C,},
	{0xE11E, 0x72CF,},
	{0xE120, 0xFFFF,},
	{0xE122, 0xE218,},
	{0xE124, 0x9009,},
	{0xE126, 0xE105,},
	{0xE128, 0x73CF,},
	{0xE12A, 0xFF00,},
	{0xE12C, 0x2FD0,},
	{0xE12E, 0x7822,},
	{0xE130, 0x7910,},
	{0xE132, 0xB202,},
	{0xE134, 0x1382,},
	{0xE136, 0x0700,},
	{0xE138, 0x0815,},
	{0xE13A, 0x03DE,},
	{0xE13C, 0x1387,},
	{0xE13E, 0x0700,},
	{0xE140, 0x2102,},
	{0xE142, 0x000A,},
	{0xE144, 0x212F,},
	{0xE146, 0x0288,},
	{0xE148, 0x1A04,},
	{0xE14A, 0x0284,},
	{0xE14C, 0x13B9,},
	{0xE14E, 0x0700,},
	{0xE150, 0xB8C1,},
	{0xE152, 0x0815,},
	{0xE154, 0x0052,},
	{0xE156, 0xDB00,},
	{0xE158, 0x230F,},
	{0xE15A, 0x0003,},
	{0xE15C, 0x2102,},
	{0xE15E, 0x00C0,},
	{0xE160, 0x7910,},
	{0xE162, 0xB202,},
	{0xE164, 0x9507,},
	{0xE166, 0x7822,},
	{0xE168, 0xE080,},
	{0xE16A, 0xD900,},
	{0xE16C, 0x20CA,},
	{0xE16E, 0x004B,},
	{0xE170, 0xB805,},
	{0xE172, 0x9533,},
	{0xE174, 0x7815,},
	{0xE176, 0x6038,},
	{0xE178, 0x0FB2,},
	{0xE17A, 0x0560,},
	{0xE17C, 0xB861,},
	{0xE17E, 0xB711,},
	{0xE180, 0x0775,},
	{0xE182, 0x0540,},
	{0xE184, 0xD900,},
	{0xE186, 0xF00A,},
	{0xE188, 0x70CF,},
	{0xE18A, 0xFFFF,},
	{0xE18C, 0xE210,},
	{0xE18E, 0x7835,},
	{0xE190, 0x8041,},
	{0xE192, 0x8000,},
	{0xE194, 0xE102,},
	{0xE196, 0xA040,},
	{0xE198, 0x09F1,},
	{0xE19A, 0x8094,},
	{0xE19C, 0x7FE0,},
	{0xE19E, 0xD800,},
	{0xE1A0, 0xC0F1,},
	{0xE1A2, 0xC5E1,},
	{0xE1A4, 0x71CF,},
	{0xE1A6, 0x0000,},
	{0xE1A8, 0x45E6,},
	{0xE1AA, 0x7960,},
	{0xE1AC, 0x7508,},
	{0xE1AE, 0x70CF,},
	{0xE1B0, 0xFFFF,},
	{0xE1B2, 0xC84C,},
	{0xE1B4, 0x9002,},
	{0xE1B6, 0x083D,},
	{0xE1B8, 0x021E,},
	{0xE1BA, 0x0D39,},
	{0xE1BC, 0x10D1,},
	{0xE1BE, 0x70CF,},
	{0xE1C0, 0xFF00,},
	{0xE1C2, 0x3354,},
	{0xE1C4, 0x9055,},
	{0xE1C6, 0x71CF,},
	{0xE1C8, 0xFFFF,},
	{0xE1CA, 0xC5D4,},
	{0xE1CC, 0x116C,},
	{0xE1CE, 0x0103,},
	{0xE1D0, 0x1170,},
	{0xE1D2, 0x00C1,},
	{0xE1D4, 0xE381,},
	{0xE1D6, 0x22C6,},
	{0xE1D8, 0x0F81,},
	{0xE1DA, 0x0000,},
	{0xE1DC, 0x00FF,},
	{0xE1DE, 0x22C4,},
	{0xE1E0, 0x0F82,},
	{0xE1E2, 0xFFFF,},
	{0xE1E4, 0x00FF,},
	{0xE1E6, 0x29C0,},
	{0xE1E8, 0x0222,},
	{0xE1EA, 0x7945,},
	{0xE1EC, 0x7930,},
	{0xE1EE, 0xB035,},
	{0xE1F0, 0x0715,},
	{0xE1F2, 0x0540,},
	{0xE1F4, 0xD900,},
	{0xE1F6, 0xF00A,},
	{0xE1F8, 0x70CF,},
	{0xE1FA, 0xFFFF,},
	{0xE1FC, 0xE224,},
	{0xE1FE, 0x7835,},
	{0xE200, 0x8041,},
	{0xE202, 0x8000,},
	{0xE204, 0xE102,},
	{0xE206, 0xA040,},
	{0xE208, 0x09F1,},
	{0xE20A, 0x8094,},
	{0xE20C, 0x7FE0,},
	{0xE20E, 0xD800,},
	{0xE210, 0xFFFF,},
	{0xE212, 0xCB40,},
	{0xE214, 0xFFFF,},
	{0xE216, 0xE0BC,},
	{0xE218, 0x0000,},
	{0xE21A, 0x0000,},
	{0xE21C, 0x0000,},
	{0xE21E, 0x0000,},
	{0xE220, 0x0000,},
	{0x098E, 0x0000,}, 	// LOGICAL_ADDRESS_ACCESS
	{0xE000, 0x1184,}, 	// PATCHLDR_LOADER_ADDRESS
	{0xE002, 0x1204,}, 	// PATCHLDR_PATCH_ID
	{0xE004, 0x4103,},//0202 	// PATCHLDR_FIRMWARE_ID
	{0xE006, 0x0202,},//0202 	// PATCHLDR_FIRMWARE_ID
	{0x0080, 0xFFF0,}, 	// COMMAND_REGISTER
	//  POLL  COMMAND_REGISTER::HOST_COMMAND_0 =>  0x00
	//  POLL  COMMAND_REGISTER::HOST_COMMAND_0 =>  0x00
	
};
static struct msm_camera_i2c_reg_conf mt9m114_recommend_settings2[] = {


	{0x0080, 0xFFF1}, 	// COMMAND_REGISTER


	

};
static struct msm_camera_i2c_reg_conf mt9m114_recommend_settings3[] = {
// AWB Start point
	{0x098E,0x2c12,},
	{0xac12,0x008f,},
	{0xac14,0x0105,},

	//[Step4-APGA //LSC]
	//  Lens register settings for A-1040SOC (MT9M114) REV2
	{0x3640, 0x00B0, },//  P_G1_P0Q0
	{0x3642, 0x8B8A, },//  P_G1_P0Q1
	{0x3644, 0x39B0, },//  P_G1_P0Q2
	{0x3646, 0xF508, },//  P_G1_P0Q3
	{0x3648, 0x9FED, },//  P_G1_P0Q4
	{0x364A, 0x01B0, },//  P_R_P0Q0
	{0x364C, 0xE129, },//  P_R_P0Q1
	{0x364E, 0x6AB0, },//  P_R_P0Q2
	{0x3650, 0xA9CC, },//  P_R_P0Q3
	{0x3652, 0x990F, },//  P_R_P0Q4
	{0x3654, 0x0150, },//  P_B_P0Q0
	{0x3656, 0x418A, },//  P_B_P0Q1
	{0x3658, 0x2410, },//  P_B_P0Q2
	{0x365A, 0xAFAB, },//  P_B_P0Q3
	{0x365C, 0xE16E, },//  P_B_P0Q4
	{0x365E, 0x00B0, },//  P_G2_P0Q0
	{0x3660, 0x8EAA, },//  P_G2_P0Q1
	{0x3662, 0x3990, },//  P_G2_P0Q2
	{0x3664, 0x9BE8, },//  P_G2_P0Q3
	{0x3666, 0xA08D, },//  P_G2_P0Q4
	{0x3680, 0x700A, },//  P_G1_P1Q0
	{0x3682, 0x3DEB, },//  P_G1_P1Q1
	{0x3684, 0x024C, },//  P_G1_P1Q2
	{0x3686, 0x1FEC, },//  P_G1_P1Q3
	{0x3688, 0x85AD, },//  P_G1_P1Q4
	{0x368A, 0xC909, },//  P_R_P1Q0
	{0x368C, 0x08EC, },//  P_R_P1Q1
	{0x368E, 0x770B, },//  P_R_P1Q2
	{0x3690, 0x598C, },//  P_R_P1Q3
	{0x3692, 0x6FAE, },//  P_R_P1Q4
	{0x3694, 0x716A, },//  P_B_P1Q0
	{0x3696, 0xE389, },//  P_B_P1Q1
	{0x3698, 0x094C, },//  P_B_P1Q2
	{0x369A, 0xF12B, },//  P_B_P1Q3
	{0x369C, 0xB28A, },//  P_B_P1Q4
	{0x369E, 0x6ECA, },//  P_G2_P1Q0
	{0x36A0, 0x43CB, },//  P_G2_P1Q1
	{0x36A2, 0x048C, },//  P_G2_P1Q2
	{0x36A4, 0x1BEC, },//  P_G2_P1Q3
	{0x36A6, 0x8A2D, },//  P_G2_P1Q4
	{0x36C0, 0x0711, },//  P_G1_P2Q0
	{0x36C2, 0xAC4E, },//  P_G1_P2Q1
	{0x36C4, 0xD831, },//  P_G1_P2Q2
	{0x36C6, 0x4570, },//  P_G1_P2Q3
	{0x36C8, 0x3312, },//  P_G1_P2Q4
	{0x36CA, 0x0491, },//  P_R_P2Q0
	{0x36CC, 0xC6EE, },//  P_R_P2Q1
	{0x36CE, 0xC0F1, },//  P_R_P2Q2
	{0x36D0, 0x2D50, },//  P_R_P2Q3
	{0x36D2, 0x33D2, },//  P_R_P2Q4
	{0x36D4, 0x3530, },//  P_B_P2Q0
	{0x36D6, 0xAF4D, },//  P_B_P2Q1
	{0x36D8, 0xC5B0, },//  P_B_P2Q2
	{0x36DA, 0x25AF, },//  P_B_P2Q3
	{0x36DC, 0x13F1, },//  P_B_P2Q4
	{0x36DE, 0x06D1, },//  P_G2_P2Q0
	{0x36E0, 0xB10E, },//  P_G2_P2Q1
	{0x36E2, 0xD351, },//  P_G2_P2Q2
	{0x36E4, 0x4930, },//  P_G2_P2Q3
	{0x36E6, 0x2CF2, },//  P_G2_P2Q4
	{0x3700, 0x4F4D, },//  P_G1_P3Q0
	{0x3702, 0x2D29, },//  P_G1_P3Q1
	{0x3704, 0x85ED, },//  P_G1_P3Q2
	{0x3706, 0xC4EA, },//  P_G1_P3Q3
	{0x3708, 0x41D1, },//  P_G1_P3Q4
	{0x370A, 0x262C, },//  P_R_P3Q0
	{0x370C, 0xADCA, },//  P_R_P3Q1
	{0x370E, 0x2B10, },//  P_R_P3Q2
	{0x3710, 0x9C8E, },//  P_R_P3Q3
	{0x3712, 0xBF0F, },//  P_R_P3Q4
	{0x3714, 0x0C0D, },//  P_B_P3Q0
	{0x3716, 0x226B, },//  P_B_P3Q1
	{0x3718, 0x078E, },//  P_B_P3Q2
	{0x371A, 0x5A8E, },//  P_B_P3Q3
	{0x371C, 0xB42D, },//  P_B_P3Q4
	{0x371E, 0x540D, },//  P_G2_P3Q0
	{0x3720, 0x74C8, },//  P_G2_P3Q1
	{0x3722, 0xAF0D, },//  P_G2_P3Q2
	{0x3724, 0xD5EA, },//  P_G2_P3Q3
	{0x3726, 0x49D1, },//  P_G2_P3Q4
	{0x3740, 0x8F11, },//  P_G1_P4Q0
	{0x3742, 0x2D30, },//  P_G1_P4Q1
	{0x3744, 0xA1ED, },//  P_G1_P4Q2
	{0x3746, 0xB392, },//  P_G1_P4Q3
	{0x3748, 0x42D5, },//  P_G1_P4Q4
	{0x374A, 0xD6CF, },//  P_R_P4Q0
	{0x374C, 0x4930, },//  P_R_P4Q1
	{0x374E, 0x8392, },//  P_R_P4Q2
	{0x3750, 0xA012, },//  P_R_P4Q3
	{0x3752, 0x7475, },//  P_R_P4Q4
	{0x3754, 0xD72D, },//  P_B_P4Q0
	{0x3756, 0x63AF, },//  P_B_P4Q1
	{0x3758, 0xA7F2, },//  P_B_P4Q2
	{0x375A, 0xAB91, },//  P_B_P4Q3
	{0x375C, 0x4795, },//  P_B_P4Q4
	{0x375E, 0x8D11, },//  P_G2_P4Q0
	{0x3760, 0x3210, },//  P_G2_P4Q1
	{0x3762, 0xB44F, },//  P_G2_P4Q2
	{0x3764, 0xB6B2, },//  P_G2_P4Q3
	{0x3766, 0x48B5, },//  P_G2_P4Q4
	{0x3784, 0x0280, },//
	{0x3782, 0x01E0, },//
	{0x37C0, 0xF8A2, },//  P_GR_Q5
	{0x37C2, 0xB8EA, },//  P_RD_Q5
	{0x37C4, 0xBB69, },//  P_BL_Q5
	{0x37C6, 0x2E23, },//  P_GB_Q5
	{0x098E, 0x0000, },//
	{0xC960, 0x0AA0, },//
	{0xC962, 0x7380, },//  CAM_PGA_L_CONFIG_GREEN_RED_Q14
	{0xC964, 0x5748, },//  CAM_PGA_L_CONFIG_RED_Q14
	{0xC966, 0x73AC, },//  CAM_PGA_L_CONFIG_GREEN_BLUE_Q14
	{0xC968, 0x6DC4, },//  CAM_PGA_L_CONFIG_BLUE_Q14
	{0xC96A, 0x0FF0, },//  CAM_PGA_M_CONFIG_COLOUR_TEMP
	{0xC96C, 0x7FFE, },//  CAM_PGA_M_CONFIG_GREEN_RED_Q14
	{0xC96E, 0x7E8E, },//  CAM_PGA_M_CONFIG_RED_Q14
	{0xC970, 0x8003, },//  CAM_PGA_M_CONFIG_GREEN_BLUE_Q14
	{0xC972, 0x7F45, },//  CAM_PGA_M_CONFIG_BLUE_Q14
	{0xC974, 0x1964, },//  CAM_PGA_R_CONFIG_COLOUR_TEMP
	{0xC976, 0x7E8D, },//  CAM_PGA_R_CONFIG_GREEN_RED_Q14
	{0xC978, 0x7D72, },//  CAM_PGA_R_CONFIG_RED_Q14
	{0xC97A, 0x7E89, },//  CAM_PGA_R_CONFIG_GREEN_BLUE_Q14
	{0xC97C, 0x7AF0, },//  CAM_PGA_R_CONFIG_BLUE_Q14
	{0xC95E, 0x0003, },//  CAM_PGA_PGA_CONTROL


	/*MIPI setting for SOC1040*/
	{0x3C5A, 0x0009,},
	{0x3C44, 0x0080,},/*MIPI_CUSTOM_SHORT_PKT*/

	/*[Tuning_settings]*/
	//{0xc940,0x00C8,},  // CAM_LL_GAMMA
	{0xC87A,0x46, MSM_CAMERA_I2C_BYTE_DATA},

	/*[CCM]*/
	{0x098E, 0x0000,},
	{0xC892, 0x0267,},/*CAM_AWB_CCM_L_0*/
	{0xC894, 0xFF1A,},/*CAM_AWB_CCM_L_1*/
	{0xC896, 0xFFB3,},/*CAM_AWB_CCM_L_2*/
	{0xC898, 0xFF80,},/*CAM_AWB_CCM_L_3*/
	{0xC89A, 0x0166,},/*CAM_AWB_CCM_L_4*/
	{0xC89C, 0x0003,},/*CAM_AWB_CCM_L_5*/
	{0xC89E, 0xFF9A,},/*CAM_AWB_CCM_L_6*/
	{0xC8A0, 0xFEB4,},/*CAM_AWB_CCM_L_7*/
	{0xC8A2, 0x024D,},/*CAM_AWB_CCM_L_8*/
	{0xC8A4, 0x01BF,},/*CAM_AWB_CCM_M_0*/
	{0xC8A6, 0xFF01,},/*CAM_AWB_CCM_M_1*/
	{0xC8A8, 0xFFF3,},/*CAM_AWB_CCM_M_2*/
	{0xC8AA, 0xFF75,},/*CAM_AWB_CCM_M_3*/
	{0xC8AC, 0x0198,},/*CAM_AWB_CCM_M_4*/
	{0xC8AE, 0xFFFD,},/*CAM_AWB_CCM_M_5*/
	{0xC8B0, 0xFF9A,},/*CAM_AWB_CCM_M_6*/
	{0xC8B2, 0xFEE7,},/*CAM_AWB_CCM_M_7*/
	{0xC8B4, 0x02A8,},/*CAM_AWB_CCM_M_8*/
	{0xC8B6, 0x01D9,},/*CAM_AWB_CCM_R_0*/
	{0xC8B8, 0xFF26,},/*CAM_AWB_CCM_R_1*/
	{0xC8BA, 0xFFF3,},/*CAM_AWB_CCM_R_2*/
	{0xC8BC, 0xFFB3,},/*CAM_AWB_CCM_R_3*/
	{0xC8BE, 0x0132,},/*CAM_AWB_CCM_R_4*/
	{0xC8C0, 0xFFE8,},/*CAM_AWB_CCM_R_5*/
	{0xC8C2, 0xFFDA,},/*CAM_AWB_CCM_R_6*/
	{0xC8C4, 0xFECD,},/*CAM_AWB_CCM_R_7*/
	{0xC8C6, 0x02C2,},/*CAM_AWB_CCM_R_8*/
	{0xC8C8, 0x0075,},/*CAM_AWB_CCM_L_RG_GAIN*/
	{0xC8CA, 0x011C,},/*CAM_AWB_CCM_L_BG_GAIN*/
	{0xC8CC, 0x009A,},/*CAM_AWB_CCM_M_RG_GAIN*/
	{0xC8CE, 0x0105,},/*CAM_AWB_CCM_M_BG_GAIN*/
	{0xC8D0, 0x00A4,},/*CAM_AWB_CCM_R_RG_GAIN*/
	{0xC8D2, 0x00AC,},/*CAM_AWB_CCM_R_BG_GAIN*/
	{0xC8D4, 0x0A8C,},/*CAM_AWB_CCM_L_CTEMP*/
	{0xC8D6, 0x0F0A,},/*CAM_AWB_CCM_M_CTEMP*/
	{0xC8D8, 0x1964,},/*CAM_AWB_CCM_R_CTEMP*/

	/*[AWB]*/
	{0xC914, 0x0000,},/*CAM_STAT_AWB_CLIP_WINDOW_XSTART*/
	{0xC916, 0x0000,},/*CAM_STAT_AWB_CLIP_WINDOW_YSTART*/
	{0xC918, 0x04FF,},/*CAM_STAT_AWB_CLIP_WINDOW_XEND*/
	{0xC91A, 0x02CF,},/*CAM_STAT_AWB_CLIP_WINDOW_YEND*/
	{0xC904, 0x0033,},/*CAM_AWB_AWB_XSHIFT_PRE_ADJ*/
	{0xC906, 0x0040,},/*CAM_AWB_AWB_YSHIFT_PRE_ADJ*/
	{0xC8F2, 0x03, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_AWB_XSCALE*/
	{0xC8F3, 0x02, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_AWB_YSCALE*/
	{0xC906, 0x003C,},/*CAM_AWB_AWB_YSHIFT_PRE_ADJ*/
	{0xC8F4, 0x0000,},/*CAM_AWB_AWB_WEIGHTS_0*/
	{0xC8F6, 0x0000,},/*CAM_AWB_AWB_WEIGHTS_1*/
	{0xC8F8, 0x0000,},/*CAM_AWB_AWB_WEIGHTS_2*/
	{0xC8FA, 0xE724,},/*CAM_AWB_AWB_WEIGHTS_3*/
	{0xC8FC, 0x1583,},/*CAM_AWB_AWB_WEIGHTS_4*/
	{0xC8FE, 0x2045,},/*CAM_AWB_AWB_WEIGHTS_5*/
	{0xC900, 0x03FF,},/*CAM_AWB_AWB_WEIGHTS_6*/
	{0xC902, 0x007C,},/*CAM_AWB_AWB_WEIGHTS_7*/
	{0xC90C, 0x80, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_K_R_L*/
	{0xC90D, 0x80, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_K_G_L*/
	{0xC90E, 0x80, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_K_B_L*/
	{0xC90F, 0x88, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_K_R_R*/
	{0xC910, 0x80, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_K_G_R*/
	{0xC911, 0x80, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AWB_K_B_R*/

	/*[Step7-CPIPE_Preference]*/
	{0xC926, 0x0020,},/*CAM_LL_START_BRIGHTNESS*/
	{0xC928, 0x009A,},/*CAM_LL_STOP_BRIGHTNESS*/
	{0xC946, 0x0070,},/*CAM_LL_START_GAIN_METRIC*/
	{0xC948, 0x00F3,},/*CAM_LL_STOP_GAIN_METRIC*/
	{0xC952, 0x0020,},/*CAM_LL_START_TARGET_LUMA_BM*/
	{0xC954, 0x009A,},/*CAM_LL_STOP_TARGET_LUMA_BM*/
	{0xC92A, 0x80, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_SATURATION*/
	{0xC92B, 0x4B, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_END_SATURATION*/
	{0xC92C, 0x00, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_DESATURATION*/
	{0xC92D, 0xFF, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_END_DESATURATION*/
	{0xC92E, 0x3C, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_DEMOSAIC*/
	{0xC92F, 0x02, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_AP_GAIN*/
	{0xC930, 0x06, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_AP_THRESH*/
	{0xC931, 0x64, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_DEMOSAIC*/
	{0xC932, 0x01, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_AP_GAIN*/
	{0xC933, 0x0C, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_AP_THRESH*/
	{0xC934, 0x3C, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_NR_RED*/
	{0xC935, 0x3C, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_NR_GREEN*/
	{0xC936, 0x3C, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_NR_BLUE*/
	{0xC937, 0x0F, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_NR_THRESH*/
	{0xC938, 0x64, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_NR_RED*/
	{0xC939, 0x64, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_NR_GREEN*/
	{0xC93A, 0x64, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_NR_BLUE*/
	{0xC93B, 0x32, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_NR_THRESH*/
	{0xC93C, 0x0020,},/*CAM_LL_START_CONTRAST_BM*/
	{0xC93E, 0x009A,},/*CAM_LL_STOP_CONTRAST_BM*/
	{0xC940, 0x00C8,},/*CAM_LL_GAMMA*/ // 0x00DC-> 0x00C8, Edison, 2012-6-20 15:20
	/*CAM_LL_START_CONTRAST_GRADIENT*/
	{0xC942, 0x38, MSM_CAMERA_I2C_BYTE_DATA},
	/*CAM_LL_STOP_CONTRAST_GRADIENT*/
	{0xC943, 0x30, MSM_CAMERA_I2C_BYTE_DATA},
	{0xC944, 0x50, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_START_CONTRAST_LUMA*/
	{0xC945, 0x19, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_STOP_CONTRAST_LUMA*/
	{0xC94A, 0x0230,},/*CAM_LL_START_FADE_TO_BLACK_LUMA*/
	{0xC94C, 0x0010,},/*CAM_LL_STOP_FADE_TO_BLACK_LUMA*/
	{0xC94E, 0x01CD,},/*CAM_LL_CLUSTER_DC_TH_BM*/
	{0xC950, 0x05, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_CLUSTER_DC_GATE*/
	{0xC951, 0x40, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_LL_SUMMING_SENSITIVITY*/
	/*CAM_AET_TARGET_AVERAGE_LUMA_DARK*/
	{0xC87B, 0x1B, MSM_CAMERA_I2C_BYTE_DATA},
	{0xC878, 0x0E, MSM_CAMERA_I2C_BYTE_DATA},/*CAM_AET_AEMODE*/
	{0xC890, 0x0080,},/*CAM_AET_TARGET_GAIN*/
	{0xC886, 0x0100,},/*CAM_AET_AE_MAX_VIRT_AGAIN*/
	{0xC87C, 0x005A,},/*CAM_AET_BLACK_CLIPPING_TARGET*/
	{0xB42A, 0x05, MSM_CAMERA_I2C_BYTE_DATA},/*CCM_DELTA_GAIN*/
	/*AE_TRACK_AE_TRACKING_DAMPENING*/
	{0xA80A, 0x20, MSM_CAMERA_I2C_BYTE_DATA},
	{0x3C44, 0x0080,},

	{0x098E, 0x0000,}, 
	{0xC984, 0x8001,},
	/*mipi_timing_t_hs_zero = 3840*/
	{0xC988, 0x0F00,},
	/*mipi_timing_t_hs_exit_hs_trail = 2823*/
	{0xC98A, 0x0B07,},
	/*mipi_timing_t_clk_post_clk_pre = 3329*/
	{0xC98C, 0x0D01,},
	/*mipi_timing_t_clk_trail_clk_zero = 1821*/
	{0xC98E, 0x071D,},
	/*mipi_timing_t_lpx = 6*/
	{0xC990, 0x0006,},
	/*mipi_timing_init_timing = 2572*/
	{0xC992, 0x0A0C,},
	{0x3C40, 0x0004, MSM_CAMERA_I2C_UNSET_WORD_MASK},
	{0xA802, 0x08, MSM_CAMERA_I2C_SET_BYTE_MASK},
	{0xC908, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0xC879, 0x01, MSM_CAMERA_I2C_BYTE_DATA},
	{0xC909, 0x01, MSM_CAMERA_I2C_UNSET_BYTE_MASK},
	{0xA80A, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0xA80B, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0xAC16, 0x18, MSM_CAMERA_I2C_BYTE_DATA},
	{0xC878, 0x08, MSM_CAMERA_I2C_SET_BYTE_MASK},
	{0xBC02, 0x08, MSM_CAMERA_I2C_UNSET_BYTE_MASK},

// optimization, 120828, Edison
// Saturation
	{0xC92A,0x84, MSM_CAMERA_I2C_BYTE_DATA}, //0x840xC0
	{0xC92B,0xe0, MSM_CAMERA_I2C_BYTE_DATA}, //0x460x60,0x46->0x82-120829//b0 c0//e0
	{0xC92D,0x01, MSM_CAMERA_I2C_BYTE_DATA}, //0xC0->0x60-120829//40 //01

	{0xAC06, 0x64, MSM_CAMERA_I2C_BYTE_DATA}, 	// AWB_R_RATIO_LOWER, 2012-8-30 1:02
	{0xAC08, 0x64, MSM_CAMERA_I2C_BYTE_DATA}, 	// AWB_B_RATIO_LOWER, 2012-8-30 1:02

// Target Gain
	{0x098E,0x4890, },
	{0xC890,0x0040, },

	{0xC942,0x3C, MSM_CAMERA_I2C_BYTE_DATA},
	{0xC944,0x4C, MSM_CAMERA_I2C_BYTE_DATA},

	{0xBC02,0x0007, },//f
	{0xC94A,0x0040, },
	{0xC94C,0x0005, },
};

static struct v4l2_subdev_info mt9m114_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_reg_conf mt9m114_config_change_settings[] = {
	{0xdc00, 0x28, MSM_CAMERA_I2C_BYTE_DATA, MSM_CAMERA_I2C_CMD_WRITE},
	{MT9M114_COMMAND_REGISTER, MT9M114_COMMAND_REGISTER_SET_STATE,
		MSM_CAMERA_I2C_UNSET_WORD_MASK, MSM_CAMERA_I2C_CMD_POLL},
	{MT9M114_COMMAND_REGISTER, (MT9M114_COMMAND_REGISTER_OK |
		MT9M114_COMMAND_REGISTER_SET_STATE), MSM_CAMERA_I2C_WORD_DATA,
		MSM_CAMERA_I2C_CMD_WRITE},
	{MT9M114_COMMAND_REGISTER, MT9M114_COMMAND_REGISTER_SET_STATE,
		MSM_CAMERA_I2C_UNSET_WORD_MASK, MSM_CAMERA_I2C_CMD_POLL},
	{0xDC01, 0x31, MSM_CAMERA_I2C_BYTE_DATA},
};

static void mt9m114_stop_stream(struct msm_sensor_ctrl_t *s_ctrl) {}

static struct msm_camera_i2c_conf_array mt9m114_init_conf[] = {
	{mt9m114_recommend_settings1,
	ARRAY_SIZE(mt9m114_recommend_settings1), 50, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_recommend_settings2,
	ARRAY_SIZE(mt9m114_recommend_settings2), 50, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_recommend_settings3,
	ARRAY_SIZE(mt9m114_recommend_settings3), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_config_change_settings,
	ARRAY_SIZE(mt9m114_config_change_settings),
	0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_conf_array mt9m114_confs[] = {
	{mt9m114_720p_settings,
	ARRAY_SIZE(mt9m114_720p_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

static struct msm_camera_i2c_reg_conf mt9m114_saturation[][1] = {
	{{0xCC12, 0x00},},
	{{0xCC12, 0x1A},},
	{{0xCC12, 0x34},},
	{{0xCC12, 0x4E},},
	{{0xCC12, 0x68},},
	{{0xCC12, 0x80},},
	{{0xCC12, 0x9A},},
	{{0xCC12, 0xB4},},
	{{0xCC12, 0xCE},},
	{{0xCC12, 0xE8},},
	{{0xCC12, 0xFF},},
};

static struct msm_camera_i2c_reg_conf mt9m114_refresh[] = {
	{MT9M114_COMMAND_REGISTER, MT9M114_COMMAND_REGISTER_REFRESH,
		MSM_CAMERA_I2C_UNSET_WORD_MASK, MSM_CAMERA_I2C_CMD_POLL},
	{MT9M114_COMMAND_REGISTER, (MT9M114_COMMAND_REGISTER_OK |
		MT9M114_COMMAND_REGISTER_REFRESH), MSM_CAMERA_I2C_WORD_DATA,
		MSM_CAMERA_I2C_CMD_WRITE},
	{MT9M114_COMMAND_REGISTER, MT9M114_COMMAND_REGISTER_REFRESH,
		MSM_CAMERA_I2C_UNSET_WORD_MASK, MSM_CAMERA_I2C_CMD_POLL},
	{MT9M114_COMMAND_REGISTER, MT9M114_COMMAND_REGISTER_OK,
		MSM_CAMERA_I2C_SET_WORD_MASK, MSM_CAMERA_I2C_CMD_POLL},
};

static struct msm_camera_i2c_conf_array mt9m114_saturation_confs[][2] = {
	{{mt9m114_saturation[0],
		ARRAY_SIZE(mt9m114_saturation[0]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[1],
		ARRAY_SIZE(mt9m114_saturation[1]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[2],
		ARRAY_SIZE(mt9m114_saturation[2]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[3],
		ARRAY_SIZE(mt9m114_saturation[3]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[4],
		ARRAY_SIZE(mt9m114_saturation[4]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[5],
		ARRAY_SIZE(mt9m114_saturation[5]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[6],
		ARRAY_SIZE(mt9m114_saturation[6]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[7],
		ARRAY_SIZE(mt9m114_saturation[7]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[8],
		ARRAY_SIZE(mt9m114_saturation[8]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[9],
		ARRAY_SIZE(mt9m114_saturation[9]), 0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
	{{mt9m114_saturation[10],
		ARRAY_SIZE(mt9m114_saturation[10]),
		0, MSM_CAMERA_I2C_WORD_DATA},
	{mt9m114_refresh,
		ARRAY_SIZE(mt9m114_refresh), 0, MSM_CAMERA_I2C_WORD_DATA},},
};

static int mt9m114_saturation_enum_map[] = {
	MSM_V4L2_SATURATION_L0,
	MSM_V4L2_SATURATION_L1,
	MSM_V4L2_SATURATION_L2,
	MSM_V4L2_SATURATION_L3,
	MSM_V4L2_SATURATION_L4,
	MSM_V4L2_SATURATION_L5,
	MSM_V4L2_SATURATION_L6,
	MSM_V4L2_SATURATION_L7,
	MSM_V4L2_SATURATION_L8,
	MSM_V4L2_SATURATION_L9,
	MSM_V4L2_SATURATION_L10,
};

static struct msm_camera_i2c_enum_conf_array mt9m114_saturation_enum_confs = {
	.conf = &mt9m114_saturation_confs[0][0],
	.conf_enum = mt9m114_saturation_enum_map,
	.num_enum = ARRAY_SIZE(mt9m114_saturation_enum_map),
	.num_index = ARRAY_SIZE(mt9m114_saturation_confs),
	.num_conf = ARRAY_SIZE(mt9m114_saturation_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};

static struct msm_camera_i2c_reg_conf mt9m114_exposure[][6] = {
	{},
	{},
	{},
	{},
	{},
	
};


static struct msm_camera_i2c_conf_array mt9m114_exposure_confs[][1] = {
	{{mt9m114_exposure[0],
		ARRAY_SIZE(mt9m114_exposure[0]), 0, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_exposure[1],
		ARRAY_SIZE(mt9m114_exposure[1]), 0, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_exposure[2],
		ARRAY_SIZE(mt9m114_exposure[2]), 0, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_exposure[3],
		ARRAY_SIZE(mt9m114_exposure[3]), 0, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_exposure[4],
		ARRAY_SIZE(mt9m114_exposure[4]), 0, MSM_CAMERA_I2C_WORD_DATA},
	},
	
};

static int mt9m114_exposure_enum_map[] = {
	MSM_V4L2_EXPOSURE_N2,
	MSM_V4L2_EXPOSURE_N1,
	MSM_V4L2_EXPOSURE_D,
	MSM_V4L2_EXPOSURE_P1,
	MSM_V4L2_EXPOSURE_P2,
};


static struct msm_camera_i2c_enum_conf_array mt9m114_exposure_enum_confs = {
	.conf = &mt9m114_exposure_confs[0][0],
	.conf_enum = mt9m114_exposure_enum_map,
	.num_enum = ARRAY_SIZE(mt9m114_exposure_enum_map),
	.num_index = ARRAY_SIZE(mt9m114_exposure_confs),
	.num_conf = ARRAY_SIZE(mt9m114_exposure_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
static struct msm_camera_i2c_reg_conf mt9m114_brightness[][4] = {
	{
{0x098E, 0xC870, MSM_CAMERA_I2C_WORD_DATA}, 	// MCU_ADDRESS [AE_BASETARGET]
{0xC870, 0x96, MSM_CAMERA_I2C_BYTE_DATA}, 	// MCU_DATA_0
{0xDC00, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, 	
{0x0080, 0x8002, MSM_CAMERA_I2C_WORD_DATA}, 
},
	{
{0x098E, 0xC870, MSM_CAMERA_I2C_WORD_DATA}, 	// MCU_ADDRESS [AE_BASETARGET]
{0xC870, 0xB6, MSM_CAMERA_I2C_BYTE_DATA}, 	// MCU_DATA_0
{0xDC00, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, 	
{0x0080, 0x8002, MSM_CAMERA_I2C_WORD_DATA}, 
},
	{
{0x098E, 0xC870, MSM_CAMERA_I2C_WORD_DATA}, 	// MCU_ADDRESS [AE_BASETARGET]
{0xC870, 0xD2, MSM_CAMERA_I2C_BYTE_DATA}, 	// MCU_DATA_0
{0xDC00, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, 	
{0x0080, 0x8002, MSM_CAMERA_I2C_WORD_DATA}, 
},
	{
{0x098E, 0xC870, MSM_CAMERA_I2C_WORD_DATA}, 	// MCU_ADDRESS [AE_BASETARGET]
{0xC870, 0x00, MSM_CAMERA_I2C_BYTE_DATA}, 	// MCU_DATA_0
{0xDC00, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, 	
{0x0080, 0x8002, MSM_CAMERA_I2C_WORD_DATA}, 
},
	{
{0x098E, 0xC870, MSM_CAMERA_I2C_WORD_DATA}, 	// MCU_ADDRESS [AE_BASETARGET]
{0xC870, 0x1C, MSM_CAMERA_I2C_BYTE_DATA}, 	// MCU_DATA_0
{0xDC00, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, 	
{0x0080, 0x8002, MSM_CAMERA_I2C_WORD_DATA}, 	
},
{
{0x098E, 0xC870, MSM_CAMERA_I2C_WORD_DATA}, 	// MCU_ADDRESS [AE_BASETARGET]
{0xC870, 0x38, MSM_CAMERA_I2C_BYTE_DATA}, 	// MCU_DATA_0
{0xDC00, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, 	
{0x0080, 0x8002, MSM_CAMERA_I2C_WORD_DATA}, 	
},
{
{0x098E, 0xC870, MSM_CAMERA_I2C_WORD_DATA}, 	// MCU_ADDRESS [AE_BASETARGET]
{0xC870, 0x54, MSM_CAMERA_I2C_BYTE_DATA}, 	// MCU_DATA_0
{0xDC00, 0x28, MSM_CAMERA_I2C_BYTE_DATA}, 	
{0x0080, 0x8002, MSM_CAMERA_I2C_WORD_DATA}, 
}
	
};


static struct msm_camera_i2c_conf_array mt9m114_brightness_confs[][1] = {
	{{mt9m114_brightness[0],
		ARRAY_SIZE(mt9m114_brightness[0]), 10, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_brightness[1],
		ARRAY_SIZE(mt9m114_brightness[1]), 10, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_brightness[2],
		ARRAY_SIZE(mt9m114_brightness[2]), 10, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_brightness[3],
		ARRAY_SIZE(mt9m114_brightness[3]), 10, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_brightness[4],
		ARRAY_SIZE(mt9m114_brightness[4]), 10, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_brightness[5],
		ARRAY_SIZE(mt9m114_brightness[5]), 10, MSM_CAMERA_I2C_WORD_DATA},
	},
	{{mt9m114_brightness[6],
		ARRAY_SIZE(mt9m114_brightness[6]), 10, MSM_CAMERA_I2C_WORD_DATA},
	}
	
};

static int mt9m114_brightness_enum_map[] = {
	MSM_V4L2_BRIGHTNESS_L0,
	MSM_V4L2_BRIGHTNESS_L1,
	MSM_V4L2_BRIGHTNESS_L2,
	MSM_V4L2_BRIGHTNESS_L3,
	MSM_V4L2_BRIGHTNESS_L4,
	MSM_V4L2_BRIGHTNESS_L5,
	MSM_V4L2_BRIGHTNESS_L6,
};


static struct msm_camera_i2c_enum_conf_array mt9m114_brightness_enum_confs = {
	.conf = &mt9m114_brightness_confs[0][0],
	.conf_enum = mt9m114_brightness_enum_map,
	.num_enum = ARRAY_SIZE(mt9m114_brightness_enum_map),
	.num_index = ARRAY_SIZE(mt9m114_brightness_confs),
	.num_conf = ARRAY_SIZE(mt9m114_brightness_confs[0]),
	.data_type = MSM_CAMERA_I2C_WORD_DATA,
};

struct msm_sensor_v4l2_ctrl_info_t mt9m114_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_SATURATION,
		.min = MSM_V4L2_SATURATION_L0,
		.max = MSM_V4L2_SATURATION_L10,
		.step = 1,
		.enum_cfg_settings = &mt9m114_saturation_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N2,
		.max = MSM_V4L2_EXPOSURE_P2,
		.step = 1,
		.enum_cfg_settings = &mt9m114_exposure_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_BRIGHTNESS,
		.min = MSM_V4L2_BRIGHTNESS_L0,
		.max = MSM_V4L2_BRIGHTNESS_L6,
		.step = 1,
		.enum_cfg_settings = &mt9m114_brightness_enum_confs,
		.s_v4l2_ctrl = msm_sensor_s_ctrl_by_enum,
	},
};

static struct msm_sensor_output_info_t mt9m114_dimensions[] = {
	{
		.x_output = 0x500,
		.y_output = 0x2D0,
		.line_length_pclk = 0x500,
		.frame_length_lines = 0x2D0,
		.vt_pixel_clk = 48000000,
		.op_pixel_clk = 128000000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csid_vc_cfg mt9m114_cid_cfg[] = {
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params mt9m114_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 1,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = mt9m114_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 1,
		.settle_cnt = 0x14,
	},
};

static struct msm_camera_csi2_params *mt9m114_csi_params_array[] = {
	&mt9m114_csi_params,
	&mt9m114_csi_params,
};

static struct msm_sensor_output_reg_addr_t mt9m114_reg_addr = {
	.x_output = 0xC868,
	.y_output = 0xC86A,
	.line_length_pclk = 0xC868,
	.frame_length_lines = 0xC86A,
};

static struct msm_sensor_id_info_t mt9m114_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x2481,
};
static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};

int32_t msm_mt9m114_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	struct msm_sensor_ctrl_t *s_ctrl;
	pr_err("%s_i2c_probe called\n", client->name);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality failed\n");
		rc = -EFAULT;
		return rc;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	if (s_ctrl->sensor_i2c_client != NULL) {
		s_ctrl->sensor_i2c_client->client = client;
		if (s_ctrl->sensor_i2c_addr != 0)
			s_ctrl->sensor_i2c_client->client->addr =
				s_ctrl->sensor_i2c_addr;
	} else {
		rc = -EFAULT;
		return rc;
	}

	s_ctrl->sensordata = client->dev.platform_data;
	if (s_ctrl->sensordata == NULL) {
		pr_err("%s: NULL sensor data\n", __func__);
		return -EFAULT;
	}
	s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
			* s_ctrl->sensordata->sensor_platform_info->num_vreg, GFP_KERNEL);
	if (!s_ctrl->reg_ptr) {
		pr_err("%s: could not allocate mem for regulators\n",
			__func__);
		return -ENOMEM;
	}
	msm_camera_power_on(&s_ctrl->sensor_i2c_client->client->dev,s_ctrl->sensordata,s_ctrl->reg_ptr);
	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;
	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}
	mdelay(1);
	rc = msm_camera_request_gpio_table(s_ctrl->sensordata, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		goto request_gpio_failed;
	}
	rc = msm_camera_config_gpio_table(s_ctrl->sensordata, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}

	usleep_range(1000, 2000);

#ifdef CONFIG_SENSOR_INFO
    	msm_sensorinfo_set_front_sensor_id(s_ctrl->sensor_id_info->sensor_id);
#else
  //do nothing here
#endif	

	rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0)
		goto probe_fail;

	if (s_ctrl->sensor_eeprom_client != NULL) {
		struct msm_camera_eeprom_client *eeprom_client =
			s_ctrl->sensor_eeprom_client;
		if (eeprom_client->func_tbl.eeprom_init != NULL &&
			eeprom_client->func_tbl.eeprom_release != NULL) {
			rc = eeprom_client->func_tbl.eeprom_init(
				eeprom_client,
				s_ctrl->sensor_i2c_client->client->adapter);
			if (rc < 0)
				goto probe_fail;

			rc = msm_camera_eeprom_read_tbl(eeprom_client,
			eeprom_client->read_tbl, eeprom_client->read_tbl_size);
			eeprom_client->func_tbl.eeprom_release(eeprom_client);
			if (rc < 0)
				goto probe_fail;
		}
	}

	snprintf(s_ctrl->sensor_v4l2_subdev.name,
		sizeof(s_ctrl->sensor_v4l2_subdev.name), "%s", id->name);
	v4l2_i2c_subdev_init(&s_ctrl->sensor_v4l2_subdev, client,
		s_ctrl->sensor_v4l2_subdev_ops);

	msm_sensor_register(&s_ctrl->sensor_v4l2_subdev);
	goto power_down;
config_gpio_failed:
	msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 0);
request_gpio_failed:
	kfree(s_ctrl->reg_ptr);
enable_clk_failed:
		msm_camera_config_gpio_table(s_ctrl->sensordata, 0);
probe_fail:
	pr_err("%s_i2c_probe failed\n", client->name);
power_down:
	if (rc > 0)
		rc = 0;
	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	return rc;
}
static const struct i2c_device_id mt9m114_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&mt9m114_s_ctrl},
	{ }
};

static struct i2c_driver mt9m114_i2c_driver = {
	.id_table = mt9m114_i2c_id,
	.probe  = msm_mt9m114_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client mt9m114_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};



int32_t msm_mt9m114_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	pr_err("%s: %d\n", __func__, __LINE__);
	#ifdef CONFIG_MACH_KISKA
	s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
			* s_ctrl->sensordata->sensor_platform_info->num_vreg, GFP_KERNEL);
	if (!s_ctrl->reg_ptr) {
		pr_err("%s: could not allocate mem for regulators\n",
			__func__);
		return -ENOMEM;
	}
	#endif
#ifdef CONFIG_MACH_KISKA
	msm_camera_power_on(&s_ctrl->sensor_i2c_client->client->dev,s_ctrl->sensordata,s_ctrl->reg_ptr);
#endif	
	//msm_camera_mipi_power_on(&s_ctrl->sensor_i2c_client->client->dev);
	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;
	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}
	mdelay(1);

	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
	}
	pr_err("%s: OK\n", __func__);
	return rc;

enable_clk_failed:
		msm_camera_config_gpio_table(data, 0);

	return rc;
}


int32_t msm_mt9m114_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	//struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	pr_err("%s\n", __func__);
	rc=gpio_direction_output(76, 0);
	if (rc < 0)  
	pr_err("%s pwd gpio76  direction 0   failed\n",__func__);
	mdelay(1);
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
#ifdef CONFIG_MACH_KISKA		
	msm_camera_power_off(&s_ctrl->sensor_i2c_client->client->dev,s_ctrl->sensordata,s_ctrl->reg_ptr);
#endif	
	//msm_camera_mipi_power_off(&s_ctrl->sensor_i2c_client->client->dev);
	return 0;
}

int32_t msm_mt9m114_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	pr_err("%s    res=%d",__func__,res);
	v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
		NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
		PIX_0, ISPIF_OFF_IMMEDIATELY));
	
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		pr_err("%s  MSM_SENSOR_REG_INIT   res=%d",__func__,res);
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
	pr_err("%s  MSM_SENSOR_UPDATE_PERIODIC   res=%d",__func__,res);
		msm_sensor_write_res_settings(s_ctrl, res);
		if (s_ctrl->curr_csi_params != s_ctrl->csi_params[res]) {
			s_ctrl->curr_csi_params = s_ctrl->csi_params[res];
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSID_CFG,
				&s_ctrl->curr_csi_params->csid_params);
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
						NOTIFY_CID_CHANGE, NULL);
			mb();
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIPHY_CFG,
				&s_ctrl->curr_csi_params->csiphy_params);
			mb();
			msleep(20);
		}

		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
			output_settings[res].op_pixel_clk);
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
			PIX_0, ISPIF_ON_FRAME_BOUNDARY));
		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(30);
		pr_err("%s: %d\n", __func__, __LINE__);
	}
	pr_err("%s: %d\n", __func__, __LINE__);
	return rc;
}


static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&mt9m114_i2c_driver);
}

static struct v4l2_subdev_core_ops mt9m114_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops mt9m114_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops mt9m114_subdev_ops = {
	.core = &mt9m114_subdev_core_ops,
	.video  = &mt9m114_subdev_video_ops,
};

static struct msm_sensor_fn_t mt9m114_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = mt9m114_stop_stream,
	.sensor_setting = msm_mt9m114_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_mt9m114_sensor_power_up,
	.sensor_power_down = msm_mt9m114_sensor_power_down,
};

static struct msm_sensor_reg_t mt9m114_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = mt9m114_config_change_settings,
	.start_stream_conf_size = ARRAY_SIZE(mt9m114_config_change_settings),
	.init_settings = &mt9m114_init_conf[0],
	.init_size = ARRAY_SIZE(mt9m114_init_conf),
	.mode_settings = &mt9m114_confs[0],
	.output_settings = &mt9m114_dimensions[0],
	.num_conf = ARRAY_SIZE(mt9m114_confs),
};

static struct msm_sensor_ctrl_t mt9m114_s_ctrl = {
	.msm_sensor_reg = &mt9m114_regs,
	.msm_sensor_v4l2_ctrl_info = mt9m114_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(mt9m114_v4l2_ctrl_info),
	.sensor_i2c_client = &mt9m114_sensor_i2c_client,
	.sensor_i2c_addr = 0x90,
	.sensor_output_reg_addr = &mt9m114_reg_addr,
	.sensor_id_info = &mt9m114_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &mt9m114_csi_params_array[0],
	.msm_sensor_mutex = &mt9m114_mut,
	.sensor_i2c_driver = &mt9m114_i2c_driver,
	.sensor_v4l2_subdev_info = mt9m114_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(mt9m114_subdev_info),
	.sensor_v4l2_subdev_ops = &mt9m114_subdev_ops,
	.func_tbl = &mt9m114_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 1.26MP YUV sensor driver");
MODULE_LICENSE("GPL v2");
