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
 */

#include "msm_actuator.h"


//add by CDZ_CAM_ZTE for AF tunning
static int msm_actuator_debug_init(struct msm_actuator_ctrl_t *a_ctrl);

int32_t msm_actuator_write_focus(
	struct msm_actuator_ctrl_t *a_ctrl,
	uint16_t curr_lens_pos,
	struct damping_params_t *damping_params,
	int8_t sign_direction,
	int16_t code_boundary)
{
	int32_t rc = 0;
	int16_t next_lens_pos = 0;
	uint16_t damping_code_step = 0;
	uint16_t wait_time = 0;
	CDBG("%s called\n",__func__);
	damping_code_step = damping_params->damping_step;
	wait_time = damping_params->damping_delay;
	CDBG("%s called, wait_time= %d\n",__func__,wait_time);
	CDBG("%s called, curr_lens_pos= %d\n",__func__,curr_lens_pos);
	CDBG("%s called, code_boundary= %d\n",__func__,code_boundary);
	CDBG("%s called, sign_direction= %d\n",__func__,sign_direction);
	CDBG("%s called, damping_code_step= %d\n",__func__,damping_code_step);

	/* Write code based on damping_code_step in a loop */
	for (next_lens_pos =
		curr_lens_pos + (sign_direction * damping_code_step);
		(sign_direction * next_lens_pos) <=
			(sign_direction * code_boundary);
		next_lens_pos =
			(next_lens_pos +
				(sign_direction * damping_code_step))) {
		CDBG("%s called, next_lens_pos= %d\n",__func__,next_lens_pos);
		rc = a_ctrl->func_tbl.
			actuator_i2c_write(a_ctrl, next_lens_pos,
				damping_params->hw_params);
		curr_lens_pos = next_lens_pos;
		usleep(wait_time);
	}

	if (curr_lens_pos != code_boundary) {
		rc = a_ctrl->func_tbl.
			actuator_i2c_write(a_ctrl, code_boundary,
				damping_params->hw_params);
		usleep(wait_time);
	}
	return rc;
}


int32_t msm_actuator_move_focus(
	struct msm_actuator_ctrl_t *a_ctrl,
	int dir,
	int32_t num_steps)
{
	int32_t rc = 0;
	int8_t sign_dir = 0;
	uint16_t curr_scene = 0;
	uint16_t scenario_size = 0;
	uint16_t index = 0;
	uint16_t step_boundary = 0;
	uint16_t target_step_pos = 0;
	uint16_t target_lens_pos = 0;
	int16_t dest_step_pos = 0;
	uint16_t curr_lens_pos = 0;
	CDBG("%s called, dir %d, num_steps %d\n",
		__func__,
		dir,
		num_steps);

	/* Determine sign direction */
	if (dir == MOVE_NEAR)
		sign_dir = 1;
	else if (dir == MOVE_FAR)
		sign_dir = -1;
	else {
		CDBG("Illegal focus direction\n");
		rc = -EINVAL;
		return rc;
	}
	CDBG("%s called, total_steps = %d\n",__func__,a_ctrl->set_info.total_steps);
	if (NULL == a_ctrl->step_position_table) {
		pr_err("%s: Invalid step_postion_table\n", __func__);
		return -EINVAL;
	}

	/* Determine destination step position */
	dest_step_pos = a_ctrl->curr_step_pos +
		(sign_dir * num_steps);

	if (dest_step_pos < 0)
		dest_step_pos = 0;
	else if (dest_step_pos > a_ctrl->set_info.total_steps)
		dest_step_pos = a_ctrl->set_info.total_steps;

	if (dest_step_pos == a_ctrl->curr_step_pos)
		return rc;

	/* Determine scenario */
	scenario_size = a_ctrl->scenario_size[dir];
	for (index = 0; index < scenario_size; index++) {
		if (num_steps <= a_ctrl->ringing_scenario[dir][index]) {
			CDBG("ringing_scenario[%d][%d]=%d\n",dir,index,a_ctrl->ringing_scenario[dir][index]);
			curr_scene = index;
			break;
		}
	}

	curr_lens_pos = a_ctrl->step_position_table[a_ctrl->curr_step_pos];
	CDBG("curr_step_pos =%d dest_step_pos =%d curr_lens_pos=%d\n",
		a_ctrl->curr_step_pos, dest_step_pos, curr_lens_pos);

	while (a_ctrl->curr_step_pos != dest_step_pos) {
		CDBG("curr_region_index =%d\n",a_ctrl->curr_region_index);
		step_boundary =
			a_ctrl->region_params[a_ctrl->curr_region_index].
			step_bound[dir];
		CDBG("step_boundary =%d\n",step_boundary);
		CDBG("sign_dir =%d\n",sign_dir);
		if ((dest_step_pos * sign_dir) <=
			(step_boundary * sign_dir)) {
			CDBG("1\n");
			target_step_pos = dest_step_pos;
			target_lens_pos =
				a_ctrl->step_position_table[target_step_pos];
			CDBG("1::target_lens_pos =%d\n",target_lens_pos);
			curr_lens_pos = a_ctrl->func_tbl.
				actuator_write_focus(
					a_ctrl,
					curr_lens_pos,
					&(a_ctrl->damping[dir]\
						[a_ctrl->curr_region_index].
						ringing_params[curr_scene]),
					sign_dir,
					target_lens_pos);
			CDBG("1::curr_lens_pos =%d\n",curr_lens_pos);
		} else {
			target_step_pos = step_boundary;
			target_lens_pos =
				a_ctrl->step_position_table[target_step_pos];
			CDBG("2::target_lens_pos =%d\n",target_lens_pos);
				curr_lens_pos = a_ctrl->func_tbl.
				actuator_write_focus(
					a_ctrl,
					curr_lens_pos,
					&(a_ctrl->damping[dir]\
						[a_ctrl->curr_region_index].
						ringing_params[curr_scene]),
					sign_dir,
					target_lens_pos);

			a_ctrl->curr_region_index += sign_dir;
			CDBG("2::curr_lens_pos =%d\n",curr_lens_pos);
		}
		a_ctrl->curr_step_pos = target_step_pos;
	}

	return rc;
}

int32_t msm_actuator_init_table(
	struct msm_actuator_ctrl_t *a_ctrl)
{
	int16_t code_per_step = 0;
	int32_t rc = 0;
	int16_t cur_code = 0;
	int16_t step_index = 0, region_index = 0;
	uint16_t step_boundary = 0;
	CDBG("%s called\n", __func__);

	if (a_ctrl->func_tbl.actuator_set_params)
		a_ctrl->func_tbl.actuator_set_params(a_ctrl);
	CDBG("cdz::%s called,a_ctrl->set_info.total_steps=%d\n", __func__,a_ctrl->set_info.total_steps);
	/* Fill step position table */
	a_ctrl->step_position_table =
		kmalloc(sizeof(uint16_t) * (a_ctrl->set_info.total_steps + 1),
			GFP_KERNEL);
	if (NULL == a_ctrl->step_position_table) {
		pr_err("%s: step_postion_table memory alloc fail\n", __func__);
		return -EINVAL;
	}
	cur_code = a_ctrl->initial_code;
	a_ctrl->step_position_table[step_index++] = cur_code;
	CDBG("cdz::%s called,region_size=%d\n", __func__,a_ctrl->region_size);
	for (region_index = 0;
		region_index < a_ctrl->region_size;
		region_index++) {
		code_per_step =
			a_ctrl->region_params[region_index].code_per_step;
		step_boundary =
			a_ctrl->region_params[region_index].step_bound[MOVE_NEAR];
		CDBG("cdz::%s called,step_boundary=%d\n", __func__,step_boundary);
		if (a_ctrl->set_info.total_steps < step_boundary) {
			pr_err("%s: Region params / total steps mismatch\n",
				__func__);
			kfree(a_ctrl->step_position_table);
			a_ctrl->step_position_table = NULL;
			return -EINVAL;
		}
		for (; step_index <= step_boundary;
			step_index++) {
			cur_code += code_per_step;
			a_ctrl->step_position_table[step_index] = cur_code;
			CDBG("1::step_position_table[%d]= %d\n",
			step_index,
			a_ctrl->step_position_table[step_index]);
		}
	}
	CDBG("cdz::%s called,set_info.total_steps=%d\n", __func__,a_ctrl->set_info.total_steps);
	for (step_index = 0;
		step_index < a_ctrl->set_info.total_steps;
		step_index++) {
		CDBG("step_position_table[%d]= %d\n",
			step_index,
			a_ctrl->step_position_table[step_index]);
	}

	a_ctrl->curr_step_pos = 0;
	a_ctrl->curr_region_index = 0;

	return rc;
}

int32_t msm_actuator_set_default_focus(
	struct msm_actuator_ctrl_t *a_ctrl)
{
	int32_t rc = 0;
	CDBG("cdz::%s called\n", __func__);

	if (!a_ctrl->step_position_table) {
		rc = a_ctrl->func_tbl.actuator_init_table(a_ctrl);
		if (rc < 0)
			return rc;
	}

	if (a_ctrl->curr_step_pos != 0)
		rc = a_ctrl->func_tbl.actuator_move_focus(a_ctrl, MOVE_FAR,
			a_ctrl->curr_step_pos);
	else if (a_ctrl->func_tbl.actuator_init_focus)
		rc = a_ctrl->func_tbl.actuator_init_focus(a_ctrl);
	return rc;
}

int32_t msm_actuator_af_power_down(struct msm_actuator_ctrl_t *a_ctrl)
{
	int32_t rc = 0;
	CDBG("cdz::%s called\n", __func__);

	if (!a_ctrl || !a_ctrl->step_position_table) {
		LINFO("%s Actuator not initialized fully, returning",
			__func__);
		return rc;
	}

	if (a_ctrl->step_position_table[a_ctrl->curr_step_pos] !=
		a_ctrl->initial_code) {
		rc = a_ctrl->func_tbl.actuator_set_default_focus(a_ctrl);
		CDBG("cdz::%s after msm_actuator_set_default_focus\n", __func__);
	}
	kfree(a_ctrl->step_position_table);
	a_ctrl->step_position_table = NULL;
	return rc;
}

int32_t msm_actuator_config(
	struct msm_actuator_ctrl_t *a_ctrl,
	void __user *argp)
{
	struct msm_actuator_cfg_data cdata;
	int32_t rc = 0;
	if (copy_from_user(&cdata,
		(void *)argp,
		sizeof(struct msm_actuator_cfg_data)))
		return -EFAULT;
	mutex_lock(a_ctrl->actuator_mutex);
	CDBG("cdz::%s called, type %d\n", __func__, cdata.cfgtype);
	switch (cdata.cfgtype) {
	case CFG_GET_ACTUATOR_INFO:
		cdata.is_af_supported = 1;
		cdata.cfg.get_info = a_ctrl->get_info;
		if (copy_to_user((void *)argp,
				 &cdata,
				 sizeof(struct msm_actuator_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_SET_ACTUATOR_INFO:
		a_ctrl->set_info = cdata.cfg.set_info;
		rc = a_ctrl->func_tbl.actuator_init_table(a_ctrl);
		if (rc < 0)
			CDBG("%s init table failed %d\n", __func__, rc);
		break;

	case CFG_SET_DEFAULT_FOCUS:
		rc = a_ctrl->func_tbl.actuator_set_default_focus(a_ctrl);
		if (rc < 0)
			CDBG("%s move focus failed %d\n", __func__, rc);
		break;

	case CFG_MOVE_FOCUS:
		rc = a_ctrl->func_tbl.actuator_move_focus(a_ctrl,
			cdata.cfg.move.dir,
			cdata.cfg.move.num_steps);
		if (rc < 0)
			CDBG("%s move focus failed %d\n", __func__, rc);
		break;

	default:
		break;
	}
	mutex_unlock(a_ctrl->actuator_mutex);
	return rc;
}

int32_t msm_actuator_i2c_probe(
	struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	struct msm_actuator_ctrl_t *act_ctrl_t = NULL;
	CDBG("cdz::%s called\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CDBG("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	act_ctrl_t = (struct msm_actuator_ctrl_t *)(id->driver_data);
	i2c_set_clientdata(client, (void *)&act_ctrl_t->actuator_ext_ctrl);
	CDBG("%s client = %x act ctrl t = %x\n",
		__func__,
		(unsigned int) client,
		(unsigned int)&act_ctrl_t->actuator_ext_ctrl);
	act_ctrl_t->i2c_client.client = client;
	if (act_ctrl_t->i2c_addr != 0)
		act_ctrl_t->i2c_client.client->addr =
			act_ctrl_t->i2c_addr;

	/* act_ctrl_t->func_tbl.actuator_init_table(act_ctrl_t); */
	CDBG("%s succeeded\n", __func__);
	return rc;

probe_failure:
	CDBG("%s failed! rc = %d\n", __func__, rc);
	return rc;
}

int32_t msm_actuator_create_subdevice(struct msm_actuator_ctrl_t *a_ctrl,
	struct i2c_board_info const *board_info,
	struct v4l2_subdev *sdev)
{
	int32_t rc = 0;

	CDBG("cdz::%s called\n", __func__);

	/* Store the sub device in actuator structure */
	a_ctrl->sdev = sdev;

	/* Assign name for sub device */
	snprintf(sdev->name, sizeof(sdev->name), "%s", board_info->type);

	/* Initialize sub device */
	v4l2_i2c_subdev_init(sdev,
		a_ctrl->i2c_client.client,
		a_ctrl->act_v4l2_subdev_ops);

	msm_actuator_debug_init(a_ctrl);
	return rc;
}
/*The functions below are added by CDZ_CAM_ZTE for AF tunning of raw sensor(ov8820) from qual */
static int msm_actuator_set_linear_total_step(void *data, u64 val)
{
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;

	/* Update total steps */
	a_ctrl->set_info.total_steps = val;

	/* Update last region's macro boundary in region params table */
	a_ctrl->region_params[a_ctrl->region_size-1].step_bound[MOVE_NEAR] =
		val;

	/* Call init table to re-create step position table
	   based on updated total steps */
	a_ctrl->func_tbl.actuator_init_table(a_ctrl);

	CDBG("%s called, total steps = %d\n",
		__func__,
		a_ctrl->set_info.total_steps);

	return 0;
}

static int msm_actuator_af_linearity_test(void *data, u64 *val)
{
	int16_t index = 0;
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;

	a_ctrl->func_tbl.actuator_set_default_focus(a_ctrl);
	msleep(1000);

	for (index = 0; index < a_ctrl->set_info.total_steps; index++) {
		a_ctrl->func_tbl.actuator_move_focus(a_ctrl, MOVE_NEAR, 1);
		CDBG("debugfs, new loc %d\n", index);
		msleep(1000);
	}

	CDBG("debugfs moved to macro boundary\n");

	for (index = 0; index < a_ctrl->set_info.total_steps; index++) {
		a_ctrl->func_tbl.actuator_move_focus(a_ctrl, MOVE_FAR, 1);
		CDBG("debugfs, new loc %d\n", index);
		msleep(1000);
	}

	return 0;
}

static int msm_actuator_af_ring_config(void *data, u64 val)
{
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;
	CDBG("%s called val %llu\n",
		__func__,
		val);
	a_ctrl->debugfs_data->step_boundary = val & 0xFF;
	a_ctrl->debugfs_data->step_value = (val >> 8) & 0xFF;
	a_ctrl->debugfs_data->step_dir = (val >> 16) & 0x01;
	CDBG("%s called dir %d, val %d, bound %d\n",
		__func__,
		a_ctrl->debugfs_data->step_dir,
		a_ctrl->debugfs_data->step_value,
		a_ctrl->debugfs_data->step_boundary);

	return 0;
}

static int msm_actuator_af_ring(void *data, u64 *val)
{
	int i = 0;
	int dir = MOVE_NEAR;
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;
	CDBG("%s called\n", __func__);
	if (a_ctrl->debugfs_data->step_dir == 1)
		dir = MOVE_FAR;

	CDBG("%s called curr step %d step val %d, dir %d, bound %d\n",
		__func__,
		a_ctrl->curr_step_pos,
		a_ctrl->debugfs_data->step_value,
		a_ctrl->debugfs_data->step_dir,
		a_ctrl->debugfs_data->step_boundary);

	for (i = 0;
		i < a_ctrl->debugfs_data->step_boundary;
		i += a_ctrl->debugfs_data->step_value) {
		CDBG("==================================================\n");
		CDBG("i %d\n", i);
		msm_actuator_move_focus(a_ctrl,
			dir,
			a_ctrl->debugfs_data->step_value);

		CDBG("==================================================\n");
		CDBG("%s called curr step %d\n",
			__func__,
			a_ctrl->curr_step_pos);

		msleep(1000);
	}
	*val = a_ctrl->curr_step_pos;
	return 0;
}

static int msm_actuator_set_af_codestep(void *data, u64 val)
{
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;
	CDBG("%s called, val %lld\n", __func__, val);

	/* Update last region's macro boundary in region params table */
	a_ctrl->region_params[a_ctrl->region_size-1].code_per_step = val;

	/* Call init table to re-create step position table
	   based on updated total steps */
	a_ctrl->func_tbl.actuator_init_table(a_ctrl);

	CDBG("%s called, code per step = %d\n",
		__func__,
		a_ctrl->region_params[a_ctrl->region_size-1].code_per_step);
	return 0;
}

static int msm_actuator_get_af_codestep(void *data, u64 *val)
{
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;
	CDBG("%s called\n", __func__);

	*val = a_ctrl->region_params[a_ctrl->region_size-1].code_per_step;
	return 0;
}

static int msm_actuator_call_init_table(void *data, u64 *val)
{
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;
	CDBG("%s called\n", __func__);
	a_ctrl->func_tbl.actuator_init_table(a_ctrl);
	CDBG("%s end\n", __func__);
	return 0;
}

static int msm_actuator_call_default_focus(void *data, u64 *val)
{
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;
	CDBG("%s called\n", __func__);
	a_ctrl->func_tbl.actuator_set_default_focus(a_ctrl);
	msleep(3000);
	CDBG("%s end\n", __func__);
	return 0;
}

static int msm_actuator_getcurstep(void *data, u64 *val)
{
	struct msm_actuator_ctrl_t *a_ctrl = (struct msm_actuator_ctrl_t *)data;
	CDBG("%s called, cur steps %d\n",
		__func__,
		a_ctrl->curr_step_pos);
	*val = a_ctrl->curr_step_pos;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(af_linear,
	msm_actuator_af_linearity_test,
	msm_actuator_set_linear_total_step,
	"%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(af_ring,
	msm_actuator_af_ring,
	msm_actuator_af_ring_config,
	"%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(af_codeperstep,
	msm_actuator_get_af_codestep,
	msm_actuator_set_af_codestep,
	"%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(af_init,
	msm_actuator_call_init_table,
	NULL,
	"%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(af_default,
	msm_actuator_call_default_focus,
	NULL,
	"%llu\n");

DEFINE_SIMPLE_ATTRIBUTE(af_getcurstep,
	msm_actuator_getcurstep,
	NULL,
	"%llu\n");

static int msm_actuator_debug_init(struct msm_actuator_ctrl_t *a_ctrl)
{
	struct dentry *debugfs_base = debugfs_create_dir("msm_actuator", NULL);
	if (!debugfs_base)
		return -ENOMEM;

	a_ctrl->debugfs_data = kmalloc(sizeof(struct debugfs_params_t),
				       GFP_KERNEL);
	if (!a_ctrl->debugfs_data)
		return -ENOMEM;

	a_ctrl->debugfs_data->step_value = 4;
	a_ctrl->debugfs_data->step_boundary = a_ctrl->set_info.total_steps;

	if (!debugfs_create_file("af_linear",
		S_IRUGO | S_IWUSR,
		debugfs_base,
		(void *)a_ctrl,
		&af_linear))
		return -ENOMEM;

	if (!debugfs_create_file("af_ring",
		S_IRUGO | S_IWUSR,
		debugfs_base,
		(void *)a_ctrl,
		&af_ring))
		return -ENOMEM;

	if (!debugfs_create_file("af_codeperstep",
		S_IRUGO | S_IWUSR,
		debugfs_base,
		(void *)a_ctrl,
		&af_codeperstep))
		return -ENOMEM;

	if (!debugfs_create_file("af_init",
		S_IRUGO | S_IWUSR,
		debugfs_base,
		(void *)a_ctrl,
		&af_init))
		return -ENOMEM;

	if (!debugfs_create_file("af_default",
		S_IRUGO | S_IWUSR,
		debugfs_base,
		(void *)a_ctrl,
		&af_default))
		return -ENOMEM;

	if (!debugfs_create_file("af_getcurstep",
		S_IRUGO | S_IWUSR,
		debugfs_base,
		(void *)a_ctrl,
		&af_getcurstep))
		return -ENOMEM;
	return 0;
}
