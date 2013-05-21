/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
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

#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/board.h>
#include <mach/msm_bus_board.h>
#include <mach/gpiomux.h>
#include "devices.h"
#include "board-8960.h"


#ifdef CONFIG_MSM_CAMERA
#if 0
//#ifdef CONFIG_MSM_CAMERA_FLASH 
#if (defined(CONFIG_GPIO_SX150X) || defined(CONFIG_GPIO_SX150X_MODULE)) && \
	defined(CONFIG_I2C)

static struct i2c_board_info cam_expander_i2c_info[] = {
	{
		I2C_BOARD_INFO("sx1508q", 0x22),
		.platform_data = &msm8960_sx150x_data[SX150X_CAM]
	},
};

static struct msm_cam_expander_info cam_expander_info[] = {
	{
		cam_expander_i2c_info,
		MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	},
};
#endif
#endif

#ifdef CONFIG_ISPCAM
static struct gpiomux_setting gsbi7_settings[] = {

	{
	.func = GPIOMUX_FUNC_2,      /*active 1*/
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
	},

	{
	.func = GPIOMUX_FUNC_1,      /*active 1*/
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_1, /*i2c suspend*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_KEEPER,
	},
};


#endif
static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 1*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 2*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 3*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_5, /*active 4*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_6, /*active 5*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_2, /*active 6*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_3, /*active 7*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*i2c suspend*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_KEEPER,
	},

};


static struct msm_gpiomux_config msm8960_cdp_flash_configs[] = {
	{
		.gpio = 3,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
};


static struct msm_gpiomux_config msm8960_cam_common_configs[] = {
	{
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 3,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	#ifdef CONFIG_ISPCAM
     {
			.gpio = 4,
			.settings = {
				[GPIOMUX_ACTIVE]	= &gsbi7_settings[0],
				[GPIOMUX_SUSPENDED] = &cam_settings[0],
			},
	},
	#endif
	{
		.gpio = 5,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 76,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 107,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
};


static struct msm_gpiomux_config msm8960_cam_2d_configs[] = {
	{
		.gpio = 20,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
	{
		.gpio = 21,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
};
   #ifdef 	CONFIG_ISPCAM
static struct msm_gpiomux_config msm8960_ispcam_configs[] = {

	{
		.gpio	   = 32,	/* GSBI7 I2C */
		.settings = {
		    [GPIOMUX_ACTIVE] = &gsbi7_settings[1],
			[GPIOMUX_SUSPENDED] = &gsbi7_settings[2],
			
		},
	},
	{
		.gpio	   = 33,	/* GSBI7 I2C */
		.settings = {
			[GPIOMUX_ACTIVE] = &gsbi7_settings[1],
			[GPIOMUX_SUSPENDED] = &gsbi7_settings[2],
			
		},
	},
};
    #endif



#define VFE_CAMIF_TIMER1_GPIO 2
#define VFE_CAMIF_TIMER2_GPIO 3
#define VFE_CAMIF_TIMER3_GPIO_INT 4

#if 0
//#ifdef CONFIG_MSM_CAMERA_FLASH 
static struct msm_camera_sensor_strobe_flash_data strobe_flash_xenon = {
	.flash_trigger = VFE_CAMIF_TIMER2_GPIO,
	.flash_charge = VFE_CAMIF_TIMER1_GPIO,
	.flash_charge_done = VFE_CAMIF_TIMER3_GPIO_INT,
	.flash_recharge_duration = 50000,
	.irq = MSM_GPIO_TO_INT(VFE_CAMIF_TIMER3_GPIO_INT),
};



static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_EXT,
	._fsrc.ext_driver_src.led_en = VFE_CAMIF_TIMER1_GPIO,
	._fsrc.ext_driver_src.led_flash_en = VFE_CAMIF_TIMER2_GPIO,
};
#endif


static struct msm_bus_vectors cam_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_preview_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 27648000,
		.ib  = 110592000,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 154275840,
		.ib  = 617103360,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 274423680,
		.ib  = 1097694720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
};

static struct msm_bus_vectors cam_zsl_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 302071680,
		.ib  = 1208286720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
};

static struct msm_bus_paths cam_bus_client_config[] = {
	{
		ARRAY_SIZE(cam_init_vectors),
		cam_init_vectors,
	},
	{
		ARRAY_SIZE(cam_preview_vectors),
		cam_preview_vectors,
	},
	{
		ARRAY_SIZE(cam_video_vectors),
		cam_video_vectors,
	},
	{
		ARRAY_SIZE(cam_snapshot_vectors),
		cam_snapshot_vectors,
	},
	{
		ARRAY_SIZE(cam_zsl_vectors),
		cam_zsl_vectors,
	},
};

static struct msm_bus_scale_pdata cam_bus_client_pdata = {
		cam_bus_client_config,
		ARRAY_SIZE(cam_bus_client_config),
		.name = "msm_camera",
};

static struct msm_camera_device_platform_data msm_camera_csi_device_data[] = {
	{
		.csid_core = 0,
		.is_csiphy = 1,
		.is_csid   = 1,
		.is_ispif  = 1,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.csid_core = 1,
		.is_csiphy = 1,
		.is_csid   = 1,
		.is_ispif  = 1,
		.is_vpe    = 1,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.ioclk.mclk_clk_rate = 24000000,
		.ioclk.vfe_clk_rate  = 228570000,
		.csid_core = 2,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
};

#if 0 
static struct camera_vreg_t msm_8960_back_cam_vreg[] = {
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};

static struct camera_vreg_t msm_8960_front_cam_vreg[] = {
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
};
#endif

static struct gpio msm8960_common_cam_gpio[] = {
	{5, GPIOF_DIR_IN, "CAMIF_MCLK"},
	{20, GPIOF_DIR_IN, "CAMIF_I2C_DATA"},
	{21, GPIOF_DIR_IN, "CAMIF_I2C_CLK"},
};

#if 0 
static struct gpio msm8960_front_cam_gpio[] = {
	{76, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct gpio msm8960_back_cam_gpio[] = {
	{107, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct msm_gpio_set_tbl msm8960_front_cam_gpio_set_tbl[] = {
	{76, GPIOF_OUT_INIT_LOW, 1000},
	{76, GPIOF_OUT_INIT_HIGH, 4000},
};
static struct msm_gpio_set_tbl msm8960_back_cam_gpio_set_tbl[] = {
	{107, GPIOF_OUT_INIT_LOW, 1000},
	{107, GPIOF_OUT_INIT_HIGH, 4000},
};
static struct msm_camera_gpio_conf msm_8960_front_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_front_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_front_cam_gpio),
	.cam_gpio_set_tbl = msm8960_front_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_front_cam_gpio_set_tbl),
};

static struct msm_camera_gpio_conf msm_8960_back_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_back_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_back_cam_gpio),
	.cam_gpio_set_tbl = msm8960_back_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_back_cam_gpio_set_tbl),
};
#endif



#ifdef CONFIG_IMX074_ACT
static struct i2c_board_info imx074_actuator_i2c_info = {
	I2C_BOARD_INFO("imx074_act", 0x11),
};

static struct msm_actuator_info imx074_actuator_info = {
	.board_info     = &imx074_actuator_i2c_info,
	.bus_id         = MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = 0,
	.vcm_enable     = 1,
};
#endif

#ifdef CONFIG_IMX074
static struct msm_camera_sensor_flash_data flash_imx074 = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
#ifdef CONFIG_MSM_CAMERA_FLASH
	//.flash_src	= &msm_flash_src
#endif
};

static struct msm_camera_sensor_platform_info sensor_board_info_imx074 = {
	.mount_angle	= 90,
	.cam_vreg = msm_8960_back_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_back_cam_vreg),
	.gpio_conf = &msm_8960_back_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_imx074_data = {
	.sensor_name	= "imx074",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_imx074,
	.strobe_flash_data = &strobe_flash_xenon,
	.sensor_platform_info = &sensor_board_info_imx074,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
	.actuator_info = &imx074_actuator_info
};
#endif

#if 0 
static struct camera_vreg_t msm_8960_mt9m114_vreg[] = {
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};
#endif

#ifdef CONFIG_OV8820_ACT
static struct i2c_board_info ov8820_actuator_i2c_info = {
 I2C_BOARD_INFO("ov8820_act", 0x6C>>2),
 };
static struct msm_actuator_info ov8820_actuator_info = {
  .board_info     = &ov8820_actuator_i2c_info,
  .bus_id         = MSM_8960_GSBI4_QUP_I2C_BUS_ID,
  .vcm_pwd        = 0,
  .vcm_enable    = 1,
  };
#endif

#ifdef CONFIG_OV8820
static struct gpio msm8960_ov8820_cam_gpio[] = {
	{54, GPIOF_DIR_OUT, "CAM_PWD"},
	{107, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct msm_gpio_set_tbl msm8960_ov8820_cam_gpio_set_tbl[] = {
	//{54, GPIOF_OUT_INIT_LOW, 1000},
	{54, GPIOF_OUT_INIT_HIGH, 1000},
	{107, GPIOF_OUT_INIT_LOW, 1000},
	{107, GPIOF_OUT_INIT_HIGH, 4000},
};
static struct msm_camera_gpio_conf msm_8960_ov8820_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_ov8820_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_ov8820_cam_gpio),
	.cam_gpio_set_tbl = msm8960_ov8820_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_ov8820_cam_gpio_set_tbl),
};
static struct camera_vreg_t msm_8960_ov8820_vreg[] = {
//       {"mipi_csi_vdd", REG_LDO, 1200000, 1200000, 20000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	//{"cam_vio", REG_VS, 0, 0, 0},
	//{"cam_vdig", REG_LDO, 1500000, 1500000, 105000},
#ifdef CONFIG_MACH_KISKA
	{"cam_vio", REG_LDO, 1800000, 1800000, 105000},
#endif
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};

static struct msm_camera_sensor_flash_data flash_ov8820 = {
#ifdef CONFIG_FLSH_ADP1650
	.flash_type	= MSM_CAMERA_FLASH_LED,
#else
	.flash_type	= MSM_CAMERA_FLASH_NONE,		
#endif
#ifdef CONFIG_MSM_CAMERA_FLASH
	//.flash_src	= &msm_flash_src,
#endif
};
static struct msm_camera_sensor_platform_info sensor_board_info_ov8820 = {
	.mount_angle	= 90,
	.cam_vreg = msm_8960_ov8820_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_ov8820_vreg),
	.gpio_conf = &msm_8960_ov8820_cam_gpio_conf,
};
static struct msm_camera_sensor_info msm_camera_sensor_ov8820_data = {
	.sensor_name	= "ov8820",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_ov8820,
#ifdef CONFIG_MSM_CAMERA_FLASH	
	//.strobe_flash_data = &strobe_flash_xenon,
#endif
	.sensor_platform_info = &sensor_board_info_ov8820,
	.csi_if	= 1,//0:Parallel interface, 1:MIPI interface 
	.camera_type = BACK_CAMERA_2D,
#ifdef CONFIG_OV8820_ACT
	.actuator_info = &ov8820_actuator_info
#endif
};
#endif

#ifdef CONFIG_OV5640
static struct gpio msm8960_ov5640_cam_gpio[] = {
	{54, GPIOF_DIR_OUT, "CAM_PWD"},
	{107, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct msm_gpio_set_tbl msm8960_ov5640_cam_gpio_set_tbl[] = {
	{54, GPIOF_OUT_INIT_HIGH, 1000},
	{54, GPIOF_OUT_INIT_LOW, 1000},
	{107, GPIOF_OUT_INIT_HIGH, 5000},
	{107, GPIOF_OUT_INIT_LOW, 5000},
	{107, GPIOF_OUT_INIT_HIGH, 1000},
};
static struct msm_camera_gpio_conf msm_8960_ov5640_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_ov5640_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_ov5640_cam_gpio),
	.cam_gpio_set_tbl = msm8960_ov5640_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_ov5640_cam_gpio_set_tbl),
};
static struct camera_vreg_t msm_8960_ov5640_vreg[] = {
//	{"mipi_csi_vdd", REG_LDO, 1200000, 1200000, 20000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	//{"cam_vio", REG_VS, 0, 0, 0},
	//{"cam_vdig", REG_LDO, 1500000, 1500000, 105000},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};
static struct msm_camera_sensor_flash_data flash_ov5640 = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
	
 #ifdef CONFIG_MSM_CAMERA_FLASH
	//.flash_src	= &msm_flash_src,
#endif
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov5640 = {
	.mount_angle	= 90,
	.cam_vreg = msm_8960_ov5640_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_ov5640_vreg),
	.gpio_conf = &msm_8960_ov5640_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov5640_data = {
	.sensor_name	= "ov5640",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_ov5640,
#ifdef CONFIG_MSM_CAMERA_FLASH	
	//.strobe_flash_data = &strobe_flash_xenon,
#endif
	.sensor_platform_info = &sensor_board_info_ov5640,
	
	.csi_if	= 1,//0:Parallel interface, 1:MIPI interface 
	.camera_type = BACK_CAMERA_2D,
};

#endif


#ifdef CONFIG_OV7692

static struct gpio msm8960_ov7692_cam_gpio[] = {
	{53, GPIOF_DIR_OUT, "CAM_PWD"},
};
static struct msm_gpio_set_tbl msm8960_ov7692_cam_gpio_set_tbl[] = {
	{53, GPIOF_OUT_INIT_HIGH, 1000},
	{53, GPIOF_OUT_INIT_LOW, 1000},
};
static struct msm_camera_gpio_conf msm_8960_ov7692_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_ov7692_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_ov7692_cam_gpio),
	.cam_gpio_set_tbl = msm8960_ov7692_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_ov7692_cam_gpio_set_tbl),
};
static struct camera_vreg_t msm_8960_ov7692_vreg[] = {
//	{"mipi_csi_vdd", REG_LDO, 1200000, 1200000, 20000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	//{"cam_vio", REG_VS, 0, 0, 0},
	//{"cam_vdig", REG_LDO, 1500000, 1500000, 105000},
//	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};
static struct msm_camera_sensor_flash_data flash_ov7692 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov7692 = {
	.mount_angle	= 270,
	.cam_vreg = msm_8960_ov7692_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_ov7692_vreg),
	.gpio_conf = &msm_8960_ov7692_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov7692_data = {
	.sensor_name	= "ov7692",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_ov7692,
	.sensor_platform_info = &sensor_board_info_ov7692,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};

#endif

#ifdef CONFIG_OV9740

static struct gpio msm8960_ov9740_cam_gpio[] = {
	{53, GPIOF_DIR_OUT, "CAM_PWD"},
      {76, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct msm_gpio_set_tbl msm8960_ov9740_cam_gpio_set_tbl[] = {
	#if 0
	{53, GPIOF_OUT_INIT_HIGH, 1000},
	{53, GPIOF_OUT_INIT_LOW, 1000},
      {76, GPIOF_OUT_INIT_LOW, 1000},
	{76, GPIOF_OUT_INIT_HIGH, 10000},
	#else
	{53, GPIOF_OUT_INIT_HIGH, 1000},
	{53, GPIOF_OUT_INIT_LOW, 1000},
      {76, GPIOF_OUT_INIT_HIGH, 5000},
      {76, GPIOF_OUT_INIT_LOW, 5000},
	{76, GPIOF_OUT_INIT_HIGH, 1000},
	#endif
};
static struct msm_camera_gpio_conf msm_8960_ov9740_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_ov9740_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_ov9740_cam_gpio),
	.cam_gpio_set_tbl = msm8960_ov9740_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_ov9740_cam_gpio_set_tbl),
};
static struct camera_vreg_t msm_8960_ov9740_vreg[] = {
 //   {"mipi_csi_vdd", REG_LDO, 1200000, 1200000, 20000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	//{"cam_vio", REG_VS, 0, 0, 0},
	//{"cam_vdig", REG_LDO, 1500000, 1500000, 105000},
  //{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};
static struct msm_camera_sensor_flash_data flash_ov9740 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov9740 = {
	.mount_angle	= 90,
	.cam_vreg = msm_8960_ov9740_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_ov9740_vreg),
	.gpio_conf = &msm_8960_ov9740_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov9740_data = {
	.sensor_name	= "ov9740",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_ov9740,
	.sensor_platform_info = &sensor_board_info_ov9740,
	.csi_if	= 1,//0:Parallel interface, 1:MIPI interface 
	.camera_type = FRONT_CAMERA_2D,
};

#endif
#ifdef CONFIG_S5K5CAGA
static struct gpio msm8960_s5k5caga_cam_gpio[] = {
	{54, GPIOF_DIR_OUT, "CAM_PWD"},
	{107, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct msm_gpio_set_tbl msm8960_s5k5caga_cam_gpio_set_tbl[] = {
	{54, GPIOF_OUT_INIT_HIGH, 1000},
	{54, GPIOF_OUT_INIT_LOW, 1000},
	{107, GPIOF_OUT_INIT_LOW, 1000},
	{107, GPIOF_OUT_INIT_HIGH, 10000},
};
static struct msm_camera_gpio_conf msm_8960_s5k5caga_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_s5k5caga_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_s5k5caga_cam_gpio),
	.cam_gpio_set_tbl = msm8960_s5k5caga_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_s5k5caga_cam_gpio_set_tbl),
};
static struct camera_vreg_t msm_8960_s5k5caga_vreg[] = {
//	{"mipi_csi_vdd", REG_LDO, 1200000, 1200000, 20000},
	{"cam_vana", REG_LDO, 2850000, 2850000, 85600},
	//{"cam_vio", REG_VS, 0, 0, 0},
	//{"cam_vdig", REG_LDO, 1800000, 1800000, 105000},
//	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};
static struct msm_camera_sensor_flash_data flash_s5k5caga = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
	
 #ifdef CONFIG_MSM_CAMERA_FLASH
	//.flash_src	= &msm_flash_src,
#endif
};

static struct msm_camera_sensor_platform_info sensor_board_info_s5k5caga = {
	.mount_angle	= 90,
	.cam_vreg = msm_8960_s5k5caga_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_s5k5caga_vreg),
	.gpio_conf = &msm_8960_s5k5caga_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k5caga_data = {
	.sensor_name	= "s5k5caga",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_s5k5caga,
#ifdef CONFIG_MSM_CAMERA_FLASH	
	//.strobe_flash_data = &strobe_flash_xenon,
#endif
	.sensor_platform_info = &sensor_board_info_s5k5caga,
	
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
};

#endif

#ifdef CONFIG_ISPCAM
static struct gpio msm8960_common_ispcam_gpio[] = {
   {4, GPIOF_DIR_IN, "CAM_MCLK"},
	//{32, GPIOF_DIR_IN, "CAM_I2C_DATA"},
	//{33, GPIOF_DIR_IN, "CAM_I2C_CLK"},
};

static struct gpio msm8960_isp_cam_gpio[] = {
	//{107, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct msm_gpio_set_tbl msm8960_isp_cam_gpio_set_tbl[] = {
   // {107, GPIOF_OUT_INIT_LOW, 1000},
	//{107, GPIOF_OUT_INIT_HIGH, 10000},
};
static struct msm_camera_gpio_conf msm_8960_isp_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_ispcam_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_ispcam_configs),
	.cam_gpio_common_tbl = msm8960_common_ispcam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_ispcam_gpio),
	.cam_gpio_req_tbl = msm8960_isp_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_isp_cam_gpio),
	.cam_gpio_set_tbl = msm8960_isp_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_isp_cam_gpio_set_tbl),
};
static struct camera_vreg_t msm_8960_isp_vreg[] = {
	//{"mipi_csi_vdd", REG_LDO, 1200000, 1200000, 20000},
	{"cam_vdig", REG_LDO, 1800000, 1800000, 105000},
	//{"cam_vio", REG_VS, 0, 0, 0},
	//{"cam_vdig", REG_LDO, 1800000, 1800000, 105000},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};
static struct msm_camera_sensor_flash_data flash_isp = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
	
 #ifdef CONFIG_MSM_CAMERA_FLASH
	//.flash_src	= &msm_flash_src,
#endif
};

static struct msm_camera_sensor_platform_info sensor_board_info_isp = {
	.mount_angle	= 90,
	.cam_vreg = msm_8960_isp_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_isp_vreg),
	.gpio_conf = &msm_8960_isp_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_isp_data = {
	.sensor_name	= "ispcam",
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_isp,
#ifdef CONFIG_MSM_CAMERA_FLASH	
	//.strobe_flash_data = &strobe_flash_xenon,
#endif
	.sensor_platform_info = &sensor_board_info_isp,
	
	.csi_if	= 1,//0:Parallel interface, 1:MIPI interface 
	.camera_type = BACK_CAMERA_2D,
};

#endif

#if 0 
static struct msm8960_privacy_light_cfg privacy_light_info = {
	.mpp = PM8921_MPP_PM_TO_SYS(12),
};
#endif

#ifdef CONFIG_MT9M114

static struct gpio msm8960_mt9m114_cam_gpio[] = {
	{76, GPIOF_DIR_OUT, "CAM_RESET"},
};
static struct msm_gpio_set_tbl msm8960_mt9m114_cam_gpio_set_tbl[] = {
	{76, GPIOF_OUT_INIT_HIGH, 1000},
	{76, GPIOF_OUT_INIT_LOW, 5000},
	{76, GPIOF_OUT_INIT_HIGH, 4000},
};
static struct msm_camera_gpio_conf msm_8960_mt9m114_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = msm8960_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(msm8960_cam_2d_configs),
	.cam_gpio_common_tbl = msm8960_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(msm8960_common_cam_gpio),
	.cam_gpio_req_tbl = msm8960_mt9m114_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(msm8960_mt9m114_cam_gpio),
	.cam_gpio_set_tbl = msm8960_mt9m114_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(msm8960_mt9m114_cam_gpio_set_tbl),
};
static struct camera_vreg_t msm_8960_mt9m114_vreg[] = {
//	{"mipi_csi_vdd", REG_LDO, 1200000, 1200000, 20000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	//{"cam_vio", REG_VS, 0, 0, 0},
	//{"cam_vdig", REG_LDO, 1800000, 1800000, 105000},
//	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};

static struct msm_camera_sensor_flash_data flash_mt9m114 = {
	.flash_type = MSM_CAMERA_FLASH_NONE
};


static struct msm_camera_sensor_platform_info sensor_board_info_mt9m114 = {
	.mount_angle = 90,
	.cam_vreg = msm_8960_mt9m114_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_mt9m114_vreg),
	.gpio_conf = &msm_8960_mt9m114_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9m114_data = {
	.sensor_name = "mt9m114",
	.pdata = &msm_camera_csi_device_data[1],
	.flash_data = &flash_mt9m114,
	.sensor_platform_info = &sensor_board_info_mt9m114,
	.csi_if = 1,
	.camera_type = FRONT_CAMERA_2D,
};
#endif

#ifdef CONFIG_OV2720 
static struct msm_camera_sensor_flash_data flash_ov2720 = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov2720 = {
	.mount_angle	= 0,
	.cam_vreg = msm_8960_front_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_front_cam_vreg),
	.gpio_conf = &msm_8960_front_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov2720_data = {
	.sensor_name	= "ov2720",
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_ov2720,
	.sensor_platform_info = &sensor_board_info_ov2720,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
};
#endif
#ifdef CONFIG_S5K3L1YX
static struct camera_vreg_t msm_8960_s5k3l1yx_vreg[] = {
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
	{"cam_vana", REG_LDO, 2800000, 2850000, 85600},
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vaf", REG_LDO, 2800000, 2800000, 300000},
};

static struct msm_camera_sensor_flash_data flash_s5k3l1yx = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_platform_info sensor_board_info_s5k3l1yx = {
	.mount_angle  = 0,
	.cam_vreg = msm_8960_s5k3l1yx_vreg,
	.num_vreg = ARRAY_SIZE(msm_8960_s5k3l1yx_vreg),
	.gpio_conf = &msm_8960_back_cam_gpio_conf,
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k3l1yx_data = {
	.sensor_name          = "s5k3l1yx",
	.pdata                = &msm_camera_csi_device_data[0],
	.flash_data           = &flash_s5k3l1yx,
	.sensor_platform_info = &sensor_board_info_s5k3l1yx,
	.csi_if               = 1,
	.camera_type          = BACK_CAMERA_2D,
};
#endif
#if 0 
static struct pm8xxx_mpp_config_data privacy_light_on_config = {
	.type		= PM8XXX_MPP_TYPE_SINK,
	.level		= PM8XXX_MPP_CS_OUT_5MA,
	.control	= PM8XXX_MPP_CS_CTRL_MPP_LOW_EN,
};

static struct pm8xxx_mpp_config_data privacy_light_off_config = {
	.type		= PM8XXX_MPP_TYPE_SINK,
	.level		= PM8XXX_MPP_CS_OUT_5MA,
	.control	= PM8XXX_MPP_CS_CTRL_DISABLE,
};

static int32_t msm_camera_8960_ext_power_ctrl(int enable)
{
	int rc = 0;
	if (enable) {
		rc = pm8xxx_mpp_config(PM8921_MPP_PM_TO_SYS(12),
			&privacy_light_on_config);
	} else {
		rc = pm8xxx_mpp_config(PM8921_MPP_PM_TO_SYS(12),
			&privacy_light_off_config);
	}
	return rc;
}
#endif

void __init msm8960_init_cam(void)
{
	msm_gpiomux_install(msm8960_cam_common_configs,
			ARRAY_SIZE(msm8960_cam_common_configs));

	if (machine_is_msm8960_cdp()) {
		msm_gpiomux_install(msm8960_cdp_flash_configs,
			ARRAY_SIZE(msm8960_cdp_flash_configs));

//#ifdef CONFIG_MSM_CAMERA_FLASH 
#if 0
		msm_flash_src._fsrc.ext_driver_src.led_en =
			GPIO_CAM_GP_LED_EN1;
		msm_flash_src._fsrc.ext_driver_src.led_flash_en =
			GPIO_CAM_GP_LED_EN2;
		#if defined(CONFIG_I2C) && (defined(CONFIG_GPIO_SX150X) || \
		defined(CONFIG_GPIO_SX150X_MODULE))
		msm_flash_src._fsrc.ext_driver_src.expander_info =
			cam_expander_info;
		#endif
#endif
	}

	if (machine_is_msm8960_liquid()) {
#ifdef CONFIG_IMX074 
		struct msm_camera_sensor_info *s_info;
		s_info = &msm_camera_sensor_imx074_data;
		s_info->sensor_platform_info->mount_angle = 180;
#elif defined CONFIG_OV2720 
		struct msm_camera_sensor_info *s_info;
		s_info = &msm_camera_sensor_ov2720_data;
		s_info->sensor_platform_info->ext_power_ctrl =
			msm_camera_8960_ext_power_ctrl;
#endif
	}

	platform_device_register(&msm8960_device_csiphy0);
	platform_device_register(&msm8960_device_csiphy1);
	platform_device_register(&msm8960_device_csiphy2);
	platform_device_register(&msm8960_device_csid0);
	platform_device_register(&msm8960_device_csid1);
	platform_device_register(&msm8960_device_csid2);
	platform_device_register(&msm8960_device_ispif);
	platform_device_register(&msm8960_device_vfe);
	platform_device_register(&msm8960_device_vpe);
}

#ifdef CONFIG_I2C
static struct i2c_board_info msm8960_camera_i2c_boardinfo[] = {
#ifdef CONFIG_IMX074 
	{
	I2C_BOARD_INFO("imx074", 0x1A),
	.platform_data = &msm_camera_sensor_imx074_data,
	},
#endif

#ifdef CONFIG_OV8820
	{
	I2C_BOARD_INFO("ov8820", 0x6C>>1),
	.platform_data = &msm_camera_sensor_ov8820_data,
	},
#endif
#ifdef CONFIG_OV5640
	{
	I2C_BOARD_INFO("ov5640", 0x78>>1),
	.platform_data = &msm_camera_sensor_ov5640_data,
	},
#endif
#ifdef CONFIG_OV7692
	{
	I2C_BOARD_INFO("ov7692", 0x78>>2),
	.platform_data = &msm_camera_sensor_ov7692_data,
	},
#endif
#ifdef CONFIG_OV9740
	{
	I2C_BOARD_INFO("ov9740", 0x20>>1),
	.platform_data = &msm_camera_sensor_ov9740_data,
	},
#endif
#ifdef CONFIG_S5K5CAGA
	{
	I2C_BOARD_INFO("s5k5caga", 0x5A>>1),
	.platform_data = &msm_camera_sensor_s5k5caga_data,
	},
#endif
#ifdef CONFIG_OV2720
	{
	I2C_BOARD_INFO("ov2720", 0x6C),
	.platform_data = &msm_camera_sensor_ov2720_data,
	},
#endif
#ifdef CONFIG_MT9M114 
	{
	I2C_BOARD_INFO("mt9m114", 0x48),
	.platform_data = &msm_camera_sensor_mt9m114_data,
	},
#endif
#ifdef CONFIG_S5K3L1YX 
	{
	I2C_BOARD_INFO("s5k3l1yx", 0x20),
	.platform_data = &msm_camera_sensor_s5k3l1yx_data,
	},
#endif
#ifdef CONFIG_MSM_CAMERA_FLASH_SC628A
	{
	I2C_BOARD_INFO("sc628a", 0x6E),
	},
#endif
};
#ifdef CONFIG_ISPCAM
static struct i2c_board_info msm8960_camera_isp_i2c_boardinfo[] = {
	{
	I2C_BOARD_INFO("ispcam", 0x3E >> 1),
	.platform_data = &msm_camera_sensor_isp_data,
	},
};


struct msm_camera_board_info msm8960_isp_camera_board_info = {
	.board_info = msm8960_camera_isp_i2c_boardinfo,
	.num_i2c_board_info = ARRAY_SIZE(msm8960_camera_isp_i2c_boardinfo),
};
#endif

struct msm_camera_board_info msm8960_camera_board_info = {
	.board_info = msm8960_camera_i2c_boardinfo,
	.num_i2c_board_info = ARRAY_SIZE(msm8960_camera_i2c_boardinfo),
};
#endif
#endif
