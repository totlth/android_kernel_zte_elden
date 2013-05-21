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

#include "msm_actuator.h"
#include <linux/debugfs.h>

#define OV8820_TOTAL_STEPS_NEAR_TO_FAR_MAX 45

#define OV8820_AF_MSB       0x3619//0x30EC
#define OV8820_AF_LSB       0x3618//0x30ED

DEFINE_MUTEX(ov8820_act_mutex);
static int ov8820_actuator_debug_init(void);
static struct msm_actuator_ctrl_t ov8820_act_t;

static int32_t ov8820_wrapper_i2c_write(struct msm_actuator_ctrl_t *a_ctrl,
	int16_t next_lens_position, void *params)
{
   	uint8_t msb = 0, lsb = 0;
	uint8_t S3_to_0 = 0x1; 
	
	msb = next_lens_position >> 4;
	lsb =((next_lens_position & 0x000F) << 4) | S3_to_0;
	CDBG("cdznew::%s: Actuator next_lens_position =%d \n", __func__, next_lens_position);
	CDBG("cdznew::%s: Actuator MSB:0x%x, LSB:0x%x\n", __func__, msb, lsb);
	
	#if 0
	msm_camera_i2c_write(&a_ctrl->i2c_client,
	           0x361a, 
			   0xb0, 
			   MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(&a_ctrl->i2c_client,
	           0x361b, 
			   0x04, 
			   MSM_CAMERA_I2C_BYTE_DATA);
	
	msm_camera_i2c_write(&a_ctrl->i2c_client,
	           0x361c, 
			   0x07, 
			   MSM_CAMERA_I2C_BYTE_DATA);
	
	#endif
	msm_camera_i2c_write(&a_ctrl->i2c_client,
	           OV8820_AF_LSB, 
			   lsb, 
			   MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(&a_ctrl->i2c_client,
	           OV8820_AF_MSB, 
			   msb, 
			   MSM_CAMERA_I2C_BYTE_DATA);

	return next_lens_position;
}

#if 0
static uint8_t ov8820_hw_params[] = {
	0x0,
	0x5,
	0x6,
	0x8,
	0xB,
};
#endif

static uint8_t ov8820_hw_params[] = {
	0x1,
};


static uint16_t ov8820_macro_scenario[] = {
	/* MOVE_NEAR dir*/
	//4,
	OV8820_TOTAL_STEPS_NEAR_TO_FAR_MAX,
};

static uint16_t ov8820_inf_scenario[] = {
	/* MOVE_FAR dir */
	//8,
	//22,
	OV8820_TOTAL_STEPS_NEAR_TO_FAR_MAX,
};

static struct region_params_t ov8820_regions[] = {
	/* step_bound[0] - macro side boundary
	 * step_bound[1] - infinity side boundary
	 */
	/* Region 1 */
	{
		.step_bound = {2, 0},
		.code_per_step = 85,
	},
	/* Region 2 */
	{
		.step_bound = {OV8820_TOTAL_STEPS_NEAR_TO_FAR_MAX, 3},
		.code_per_step = 17,//18
	}
};

static struct damping_params_t ov8820_macro_reg1_damping[] = {
	/* MOVE_NEAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0xFF,
		.damping_delay = 0,//1500,
		.hw_params = &ov8820_hw_params[0],
	},
	#if 0
	/* Scene 2 => Damping params */
	{
		.damping_step = 0xFF,
		.damping_delay = 1500,
	//	.hw_params = &ov8820_hw_params[0],
	},
	#endif
};
#if 0

static struct damping_params_t ov8820_macro_reg2_damping[] = {
	/* MOVE_NEAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0xFF,
		.damping_delay = 4500,
	//	.hw_params = &ov8820_hw_params[4],
	},
	/* Scene 2 => Damping params */
	{
		.damping_step = 0xFF,
		.damping_delay = 4500,
		.hw_params = &ov8820_hw_params[3],
	},
};
#endif

static struct damping_params_t ov8820_inf_reg1_damping[] = {
	/* MOVE_FAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0xFF,
		.damping_delay = 0,//450,
		.hw_params = &ov8820_hw_params[0],
	},
#if 0
/* Scene 2 => Damping params */
	{
		.damping_step = 0xFF,
		.damping_delay = 450,
		.hw_params = &ov8820_hw_params[0],
	},
	/* Scene 3 => Damping params */
	{
		.damping_step = 0xFF,
		.damping_delay = 450,
		.hw_params = &ov8820_hw_params[0],
	},
#endif
};

#if 0
static struct damping_params_t ov8820_inf_reg2_damping[] = {
	/* MOVE_FAR Dir */
	/* Scene 1 => Damping params */
	{
		.damping_step = 0x1FF,
		.damping_delay = 4500,
		.hw_params = &ov8820_hw_params[2],
	},
	/* Scene 2 => Damping params */
	{
		.damping_step = 0x1FF,
		.damping_delay = 4500,
		.hw_params = &ov8820_hw_params[1],
	},
	/* Scene 3 => Damping params */
	{
		.damping_step = 27,
		.damping_delay = 2700,
		.hw_params = &ov8820_hw_params[0],
	},
};
#endif

static struct damping_t ov8820_macro_regions[] = {
	/* MOVE_NEAR dir */
	/* Region 1 */
	{
		.ringing_params = ov8820_macro_reg1_damping,
	},
	#if 0
	/* Region 2 */
	{
		.ringing_params = ov8820_macro_reg2_damping,
	},
	#endif
};

static struct damping_t ov8820_inf_regions[] = {
	/* MOVE_FAR dir */
	/* Region 1 */
	{
		.ringing_params = ov8820_inf_reg1_damping,
	},
	#if 0
	/* Region 2 */
	{
		.ringing_params = ov8820_inf_reg2_damping,
	},
	#endif
};

int32_t ov8820_act_write_focus(
	struct msm_actuator_ctrl_t *a_ctrl,
	uint16_t curr_lens_pos,
	struct damping_params_t *damping_params,
	int8_t sign_direction,
	int16_t code_boundary)
{
	int32_t rc = 0;
	
	LINFO("%s called, curr lens pos = %d, code_boundary = %d\n",
		  __func__,
		  curr_lens_pos,
		  code_boundary);

	rc = a_ctrl->func_tbl.actuator_i2c_write(a_ctrl, code_boundary, NULL);;

	return rc;
}

static int32_t ov8820_set_params(struct msm_actuator_ctrl_t *a_ctrl)
{
	return 0;
}

static const struct i2c_device_id ov8820_act_i2c_id[] = {
	{"ov8820_act", (kernel_ulong_t)&ov8820_act_t},
	{ }
};

static int ov8820_act_config(
	void __user *argp)
{
	CDBG("cdz::%s called\n", __func__);
	return (int) msm_actuator_config(&ov8820_act_t, argp);
}

static int ov8820_i2c_add_driver_table(
	void)
{
	CDBG("cdz::%s called\n", __func__);
	return (int) msm_actuator_init_table(&ov8820_act_t);
}

static struct i2c_driver ov8820_act_i2c_driver = {
	.id_table = ov8820_act_i2c_id,
	.probe  = msm_actuator_i2c_probe,
	.remove = __exit_p(ov8820_act_i2c_remove),
	.driver = {
		.name = "ov8820_act",
	},
};

static int __init ov8820_i2c_add_driver(
	void)
{
	uint16_t rc;
	CDBG("cdz::%s called\n", __func__);
	rc=i2c_add_driver(ov8820_act_t.i2c_driver);
	CDBG("cdz::%s rc=%d\n", __func__,rc);
	return rc;	
}

static struct v4l2_subdev_core_ops ov8820_act_subdev_core_ops;

static struct v4l2_subdev_ops ov8820_act_subdev_ops = {
	.core = &ov8820_act_subdev_core_ops,
};

static int32_t ov8820_act_probe(
	void *board_info,
	void *sdev)
{
       CDBG("cdz::%s called\n", __func__);
	ov8820_actuator_debug_init();

	return (int) msm_actuator_create_subdevice(&ov8820_act_t,
		(struct i2c_board_info const *)board_info,
		(struct v4l2_subdev *)sdev);
}

static struct msm_actuator_ctrl_t ov8820_act_t = {
	.i2c_driver = &ov8820_act_i2c_driver,
	.i2c_addr = 0x6C,
	.act_v4l2_subdev_ops = &ov8820_act_subdev_ops,
	.actuator_ext_ctrl = {
		.a_init_table = ov8820_i2c_add_driver_table,
		.a_create_subdevice = ov8820_act_probe,
		.a_config = ov8820_act_config,
	},

	.i2c_client = {
		.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
	},

	.set_info = {
		.total_steps = OV8820_TOTAL_STEPS_NEAR_TO_FAR_MAX,
	},

	.curr_step_pos = 0,
	.curr_region_index = 0,
	.initial_code = 0,
	.actuator_mutex = &ov8820_act_mutex,

	/* Initialize scenario */
	.ringing_scenario[MOVE_NEAR] = ov8820_macro_scenario,
	.scenario_size[MOVE_NEAR] = ARRAY_SIZE(ov8820_macro_scenario),
	.ringing_scenario[MOVE_FAR] = ov8820_inf_scenario,
	.scenario_size[MOVE_FAR] = ARRAY_SIZE(ov8820_inf_scenario),

	/* Initialize region params */
	.region_params = ov8820_regions,
	.region_size = ARRAY_SIZE(ov8820_regions),

	/* Initialize damping params */
	.damping[MOVE_NEAR] = ov8820_macro_regions,
	.damping[MOVE_FAR] = ov8820_inf_regions,

	.func_tbl = {
		.actuator_set_params = ov8820_set_params,
		.actuator_init_focus = NULL,
		.actuator_init_table = msm_actuator_init_table,
		.actuator_move_focus = msm_actuator_move_focus,
//		.actuator_write_focus = msm_actuator_write_focus,
		.actuator_write_focus = ov8820_act_write_focus,
		.actuator_set_default_focus = msm_actuator_set_default_focus,
		.actuator_i2c_write = ov8820_wrapper_i2c_write,

	},

};

static int ov8820_actuator_set_delay(void *data, u64 val)
{
	CDBG("cdz::%s called\n", __func__);
	ov8820_inf_reg1_damping[1].damping_delay = val;
	return 0;
}

static int ov8820_actuator_get_delay(void *data, u64 *val)
{
	CDBG("cdz::%s called\n", __func__);
	*val = ov8820_inf_reg1_damping[1].damping_delay;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ov8820_delay,
	ov8820_actuator_get_delay,
	ov8820_actuator_set_delay,
	"%llu\n");

static int ov8820_actuator_set_jumpparam(void *data, u64 val)
{
	CDBG("cdz::%s called\n", __func__);
	ov8820_inf_reg1_damping[1].damping_step = val & 0xFFF;
	return 0;
}

static int ov8820_actuator_get_jumpparam(void *data, u64 *val)
{
	CDBG("cdz::%s called\n", __func__);
	*val = ov8820_inf_reg1_damping[1].damping_step;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ov8820_jumpparam,
	ov8820_actuator_get_jumpparam,
	ov8820_actuator_set_jumpparam,
	"%llu\n");
#if 0 
static int ov8820_actuator_set_hwparam(void *data, u64 val)
{
	ov8820_hw_params[2] = val & 0xFF;
	pr_err("cdz::%s called\n", __func__);
	return 0;
}

static int ov8820_actuator_get_hwparam(void *data, u64 *val)
{
	*val = ov8820_hw_params[2];
	pr_err("cdz::%s called\n", __func__);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ov8820_hwparam,
	ov8820_actuator_get_hwparam,
	ov8820_actuator_set_hwparam,
	"%llu\n");
#endif
#if 1 
static int ov8820_actuator_set_hwparam(void *data, u64 val)
{
	ov8820_hw_params[0] = val & 0xFF;
	CDBG("cdz::%s called\n", __func__);
	return 0;
}

static int ov8820_actuator_get_hwparam(void *data, u64 *val)
{
	*val = ov8820_hw_params[0];
	CDBG("cdz::%s called\n", __func__);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ov8820_hwparam,
	ov8820_actuator_get_hwparam,
	ov8820_actuator_set_hwparam,
	"%llu\n");
#endif

static int ov8820_actuator_debug_init(void)
{
	struct dentry *debugfs_base = debugfs_create_dir("ov8820_actuator", NULL);
	CDBG("cdz::%s called\n", __func__);
	if (!debugfs_base)
		return -ENOMEM;

	if (!debugfs_create_file("ov8820_delay",
		S_IRUGO | S_IWUSR, debugfs_base, NULL, &ov8820_delay))
		return -ENOMEM;

	if (!debugfs_create_file("ov8820_jumpparam",
		S_IRUGO | S_IWUSR, debugfs_base, NULL, &ov8820_jumpparam))
		return -ENOMEM;
#if 1
	if (!debugfs_create_file("ov8820_hwparam",
		S_IRUGO | S_IWUSR, debugfs_base, NULL, &ov8820_hwparam))
		return -ENOMEM;
#endif
	return 0;
}
subsys_initcall(ov8820_i2c_add_driver);
MODULE_DESCRIPTION("OV8820 actuator");
MODULE_LICENSE("GPL v2");
