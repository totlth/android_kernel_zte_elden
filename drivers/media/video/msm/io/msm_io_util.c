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
 */

#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <mach/board.h>
#include <mach/camera.h>
#include <mach/gpiomux.h>






int msm_cam_clk_enable(struct device *dev, struct msm_cam_clk_info *clk_info,
		struct clk **clk_ptr, int num_clk, int enable)
{
	int i;
	int rc = 0;
	if (enable) {
		for (i = 0; i < num_clk; i++) {
			clk_ptr[i] = clk_get(dev, clk_info[i].clk_name);
			if (IS_ERR(clk_ptr[i])) {
				pr_err("%s get failed\n", clk_info[i].clk_name);
				rc = PTR_ERR(clk_ptr[i]);
				goto cam_clk_get_err;
			}
			if (clk_info[i].clk_rate >= 0) {
				rc = clk_set_rate(clk_ptr[i],
							clk_info[i].clk_rate);
				if (rc < 0) {
					pr_err("%s set failed\n",
						   clk_info[i].clk_name);
					goto cam_clk_set_err;
				}
			}
			rc = clk_enable(clk_ptr[i]);
			if (rc < 0) {
				pr_err("%s enable failed\n",
					   clk_info[i].clk_name);
				goto cam_clk_set_err;
			}
		}
	} else {
		for (i = num_clk - 1; i >= 0; i--) {
			if (clk_ptr[i] != NULL)
				clk_disable(clk_ptr[i]);
				clk_put(clk_ptr[i]);
		}
	}
	return rc;


cam_clk_set_err:
	clk_put(clk_ptr[i]);
cam_clk_get_err:
	for (i--; i >= 0; i--) {
		if (clk_ptr[i] != NULL) {
			clk_disable(clk_ptr[i]);
			clk_put(clk_ptr[i]);
		}
	}
	return rc;
}

int msm_camera_config_vreg(struct device *dev, struct camera_vreg_t *cam_vreg,
		int num_vreg, struct regulator **reg_ptr, int config)
{
	int i = 0;
	int rc = 0;
	struct camera_vreg_t *curr_vreg;
	if (config) {
		for (i = 0; i < num_vreg; i++) {
			curr_vreg = &cam_vreg[i];
			if(reg_ptr[i]==NULL){
			reg_ptr[i] = regulator_get(dev,
				curr_vreg->reg_name);
			if (IS_ERR(reg_ptr[i])) {
				pr_err("%s: %s get failed\n",
				 __func__,
				 curr_vreg->reg_name);
				reg_ptr[i] = NULL;
				goto vreg_get_fail;
			}
			if (curr_vreg->type == REG_LDO) {
				rc = regulator_set_voltage(
				reg_ptr[i],
				curr_vreg->min_voltage,
				curr_vreg->max_voltage);
				if (rc < 0) {
					pr_err(
					"%s: %s set voltage failed\n",
					__func__,
					curr_vreg->reg_name);
					goto vreg_set_voltage_fail;
				}
				if (curr_vreg->op_mode >= 0) {
					rc = regulator_set_optimum_mode(
						reg_ptr[i],
						curr_vreg->op_mode);
					if (rc < 0) {
						pr_err(
						"%s: %s set optimum mode failed\n",
						__func__,
						curr_vreg->reg_name);
						goto vreg_set_opt_mode_fail;
					}
				}
			}
		}}
	} else {
		for (i = num_vreg-1; i >= 0; i--) {
			curr_vreg = &cam_vreg[i];
			if (reg_ptr[i]) {
				if (curr_vreg->type == REG_LDO) {
					if (curr_vreg->op_mode >= 0) {
						regulator_set_optimum_mode(
							reg_ptr[i], 0);
					}
					regulator_set_voltage(
						reg_ptr[i],
						0, curr_vreg->max_voltage);
				}
				regulator_put(reg_ptr[i]);
				reg_ptr[i] = NULL;
			}
		}
	}
	return 0;

vreg_unconfig:
if (curr_vreg->type == REG_LDO)
	regulator_set_optimum_mode(reg_ptr[i], 0);

vreg_set_opt_mode_fail:
if (curr_vreg->type == REG_LDO)
	regulator_set_voltage(reg_ptr[i], 0,
		curr_vreg->max_voltage);

vreg_set_voltage_fail:
	regulator_put(reg_ptr[i]);
	reg_ptr[i] = NULL;

vreg_get_fail:
	for (i--; i >= 0; i--) {
		curr_vreg = &cam_vreg[i];
		goto vreg_unconfig;
	}
	return -ENODEV;
}

int msm_camera_enable_vreg(struct device *dev, struct camera_vreg_t *cam_vreg,
		int num_vreg, struct regulator **reg_ptr, int enable)
{
	int i = 0, rc = 0;
	if (enable) {
		for (i = 0; i < num_vreg; i++) {
			if (IS_ERR(reg_ptr[i])) {
				pr_err("%s: %s null regulator\n",
				__func__, cam_vreg[i].reg_name);
				goto disable_vreg;
			}
			rc = regulator_enable(reg_ptr[i]);
			if (rc < 0) {
				pr_err("%s: %s enable failed\n",
				__func__, cam_vreg[i].reg_name);
				goto disable_vreg;
			}
		}
	} else {
		for (i = num_vreg-1; i >= 0; i--)
			regulator_disable(reg_ptr[i]);
	}
	return rc;
disable_vreg:
	for (i--; i >= 0; i--) {
		regulator_disable(reg_ptr[i]);
		goto disable_vreg;
	}
	return rc;
}

int msm_camera_request_gpio_table(struct msm_camera_sensor_info *sinfo,
	int gpio_en)
{
	int rc = 0,i;
	struct msm_camera_gpio_conf *gpio_conf =
		sinfo->sensor_platform_info->gpio_conf;

	if (gpio_conf->cam_gpio_req_tbl == NULL ||
		gpio_conf->cam_gpio_common_tbl == NULL) {
		pr_err("%s: NULL camera gpio table\n", __func__);
		return -EFAULT;
	}

	if (gpio_en) {
		if (gpio_conf->cam_gpiomux_conf_tbl != NULL) {
			msm_gpiomux_install(
				(struct msm_gpiomux_config *)gpio_conf->
				cam_gpiomux_conf_tbl,
				gpio_conf->cam_gpiomux_conf_tbl_size);
		}
		#if 0
		rc = gpio_request_array(gpio_conf->cam_gpio_common_tbl,
				gpio_conf->cam_gpio_common_tbl_size);
		if (rc < 0) {
			pr_err("%s common gpio request failed\n", __func__);
			return rc;
		}
		
		rc = gpio_request_array(gpio_conf->cam_gpio_req_tbl,
				gpio_conf->cam_gpio_req_tbl_size);
		if (rc < 0) {
			pr_err("%s camera gpio request failed\n", __func__);
			gpio_free_array(gpio_conf->cam_gpio_common_tbl,
				gpio_conf->cam_gpio_common_tbl_size);
			return rc;
		}
		#else
		for (i = 0; i < gpio_conf->cam_gpio_common_tbl_size; i++){
			rc=gpio_request_one(gpio_conf->cam_gpio_common_tbl[i].gpio,
				gpio_conf->cam_gpio_common_tbl[i].flags, gpio_conf->cam_gpio_common_tbl[i].label);
			if(rc!=0){
			gpio_free(gpio_conf->cam_gpio_common_tbl[i].gpio);
			mdelay(1);
			rc=gpio_request_one(gpio_conf->cam_gpio_common_tbl[i].gpio,
				gpio_conf->cam_gpio_common_tbl[i].flags, gpio_conf->cam_gpio_common_tbl[i].label);
			}
		}
		for (i = 0; i < gpio_conf->cam_gpio_req_tbl_size; i++){
			rc=gpio_request_one(gpio_conf->cam_gpio_req_tbl[i].gpio,
				gpio_conf->cam_gpio_req_tbl[i].flags, gpio_conf->cam_gpio_req_tbl[i].label);
			if(rc!=0){
			gpio_free(gpio_conf->cam_gpio_req_tbl[i].gpio);
			mdelay(1);
			rc=gpio_request_one(gpio_conf->cam_gpio_req_tbl[i].gpio,
				gpio_conf->cam_gpio_req_tbl[i].flags, gpio_conf->cam_gpio_req_tbl[i].label);
			}
	
		}
		#endif
		
	} else {
		gpio_free_array(gpio_conf->cam_gpio_req_tbl,
				gpio_conf->cam_gpio_req_tbl_size);
		gpio_free_array(gpio_conf->cam_gpio_common_tbl,
			gpio_conf->cam_gpio_common_tbl_size);
	}
	return 0;
}

int msm_camera_config_gpio_table(struct msm_camera_sensor_info *sinfo,
	int gpio_en)
{
	struct msm_camera_gpio_conf *gpio_conf =
		sinfo->sensor_platform_info->gpio_conf;
	int rc = 0, i;

	if (gpio_en) {
		for (i = 0; i < gpio_conf->cam_gpio_set_tbl_size; i++) {
			gpio_set_value_cansleep(
				gpio_conf->cam_gpio_set_tbl[i].gpio,
				gpio_conf->cam_gpio_set_tbl[i].flags);
			usleep_range(gpio_conf->cam_gpio_set_tbl[i].delay,
				gpio_conf->cam_gpio_set_tbl[i].delay + 1000);
		}
	} else {
		for (i = gpio_conf->cam_gpio_set_tbl_size - 1; i >= 0; i--) {
			#if 0
			if (gpio_conf->cam_gpio_set_tbl[i].flags)
				gpio_set_value_cansleep(
					gpio_conf->cam_gpio_set_tbl[i].gpio,
					GPIOF_OUT_INIT_LOW);
			#else
			gpio_set_value_cansleep(
				gpio_conf->cam_gpio_set_tbl[i].gpio,
				gpio_conf->cam_gpio_set_tbl[i].flags);
			usleep_range(gpio_conf->cam_gpio_set_tbl[i].delay,
				gpio_conf->cam_gpio_set_tbl[i].delay + 1000);
			#endif
		}
	}
	return rc;
}

//add by lijing for mipi power 20120731
static struct regulator *mipi_csi_vdd;
int msm_camera_mipi_power_on(struct device *dev)
{
      pr_err("lijing:%s\n",__func__);
	if (mipi_csi_vdd == NULL) {
		mipi_csi_vdd = regulator_get(dev, "mipi_csi_vdd");
		if (IS_ERR(mipi_csi_vdd)) {
			pr_err("%s: VREG CAM mipi_csi_vdd get failed\n", __func__);
		}
		regulator_set_voltage(mipi_csi_vdd,1200000,1200000);
		//rc = regulator_set_optimum_mode(
						//mipi_csi_vdd,
						//20000);
		if (regulator_enable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM mipi_csi_vdd enable failed\n", __func__);
		}
	}
	return 0;
}

int msm_camera_mipi_power_off(struct device *dev)
{
       pr_err("lijing:%s\n",__func__);
	if(mipi_csi_vdd != NULL){
		regulator_disable(mipi_csi_vdd);
		//regulator_set_optimum_mode(mipi_csi_vdd, 0);
		//regulator_set_voltage(mipi_csi_vdd,0, 1200000);
		regulator_put(mipi_csi_vdd);
		mipi_csi_vdd = NULL;
	}
	return 0;
}
#if defined CONFIG_MACH_DANA  ||defined CONFIG_MACH_FROSTY||defined CONFIG_MACH_JARVIS || defined CONFIG_MACH_KISKA

static bool hasvreged=0;
//static struct regulator *mipi_csi_vdd;
#if defined (CONFIG_OV5640)||defined (CONFIG_OV7692)||defined (CONFIG_S5K5CAGA)||defined (CONFIG_MT9M114)
static struct regulator *cam_vdig;
#endif
int msm_camera_power_on(struct device *dev,struct msm_camera_sensor_info *sinfo,struct regulator **reg_ptr)
{		int rc=0;
#ifdef CONFIG_MACH_KISKA
	if(1)
#else			
	if(hasvreged==0)
#endif
	{
		pr_err("%s: %d\n", __func__, __LINE__);
		#if 0
		if (mipi_csi_vdd == NULL) {
		mipi_csi_vdd = regulator_get(dev, "mipi_csi_vdd");
		if (IS_ERR(mipi_csi_vdd)) {
			pr_err("%s: VREG CAM mipi_csi_vdd get failed\n", __func__);
			return rc;
		}
		rc = regulator_set_voltage(
				mipi_csi_vdd,
				1200000,
				1200000);
		rc = regulator_set_optimum_mode(
						mipi_csi_vdd,
						20000);
		if (regulator_enable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM mipi_csi_vdd enable failed\n", __func__);
				return rc;
			}
		}
		#endif

		rc = msm_camera_config_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: regulator on failed\n", __func__);
			goto config_vreg_failed;
		}
		
		rc = msm_camera_enable_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: enable regulator failed\n", __func__);
			goto enable_vreg_failed;
		}
		
		#if defined (CONFIG_OV5640)||defined (CONFIG_OV7692)||defined (CONFIG_S5K5CAGA)||defined (CONFIG_MT9M114) 
		if (cam_vdig == NULL) {
		cam_vdig = regulator_get(dev, "cam_vdig");
		if (IS_ERR(cam_vdig)) {
			pr_err("%s: VREG CAM VDIG get failed\n", __func__);
		}
		if (regulator_enable(cam_vdig)) {
				pr_err("%s: VREG CAM VDIG enable failed\n", __func__);
			}
		}
		
		#endif	
		#ifdef CONFIG_OV8820
		#ifdef CONFIG_MACH_KISKA
		rc = gpio_request(89,"SENSOR_NAME");
		if (!rc) {
			pr_err("%s: gpio_request 09 OK\n", __func__);
			gpio_direction_output(89,1);
			usleep_range(1000, 2000);
			mdelay(1);
		} else {
			gpio_free(89);
			mdelay(1);
			rc = gpio_request(89,"SENSOR_NAME");
			if (!rc){
			pr_err("%s: gpio_request 09 OK\n", __func__);
			gpio_direction_output(89,1);
			usleep_range(1000, 2000);
			mdelay(1);
			}else{
			pr_err("%s: gpio_request 09 failed\n", __func__);
			}
		}
		#endif
		rc = gpio_request(9,"SENSOR_NAME");
		if (!rc) {
			pr_err("%s: gpio_request 09 OK\n", __func__);
			gpio_direction_output(9,1);
			usleep_range(1000, 2000);
			mdelay(1);
		} else {
			gpio_free(9);
			mdelay(1);
			rc = gpio_request(9,"SENSOR_NAME");
			if (!rc){
			pr_err("%s: gpio_request 09 OK\n", __func__);
			gpio_direction_output(9,1);
			usleep_range(1000, 2000);
			mdelay(1);
			}else{
			pr_err("%s: gpio_request 09 failed\n", __func__);
			}
		}
		#endif
		
		
		hasvreged=1;
		return rc;
enable_vreg_failed:
	msm_camera_config_vreg(dev,
		sinfo->sensor_platform_info->cam_vreg,
		sinfo->sensor_platform_info->num_vreg,
		reg_ptr, 0);
config_vreg_failed:
	msm_camera_request_gpio_table(sinfo, 0);
	return rc;

		}else{
	
		pr_err("%s: %d\n", __func__, __LINE__);
		#if 0
		if (mipi_csi_vdd == NULL) {
		mipi_csi_vdd = regulator_get(dev, "mipi_csi_vdd");
		if (IS_ERR(mipi_csi_vdd)) {
			pr_err("%s: VREG CAM mipi_csi_vdd get failed\n", __func__);
			return rc;
		}
		rc = regulator_set_voltage(
				mipi_csi_vdd,
				1200000,
				1200000);
		rc = regulator_set_optimum_mode(
						mipi_csi_vdd,
						20000);
		if (regulator_enable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM mipi_csi_vdd enable failed\n", __func__);
				return rc;
			}
		}
		#endif


	return 0;
		}
		
}
#elif defined CONFIG_MACH_ELDEN || defined CONFIG_MACH_ILIAMNA
static bool hasvreged=0;
//static struct regulator *mipi_csi_vdd;
#if defined (CONFIG_OV5640)||defined (CONFIG_OV7692)
static struct regulator *cam_vio;
#endif
int msm_camera_power_on(struct device *dev,struct msm_camera_sensor_info *sinfo,struct regulator **reg_ptr)
{		int rc=0;
		if(hasvreged==0){
		
		pr_err("%s: %d\n", __func__, __LINE__);
		#if 0
		if (mipi_csi_vdd == NULL) {
		mipi_csi_vdd = regulator_get(dev, "mipi_csi_vdd");
		if (IS_ERR(mipi_csi_vdd)) {
			pr_err("%s: VREG CAM mipi_csi_vdd get failed\n", __func__);
			return rc;
		}
		rc = regulator_set_voltage(
				mipi_csi_vdd,
				1200000,
				1200000);
		rc = regulator_set_optimum_mode(
						mipi_csi_vdd,
						20000);
		if (regulator_enable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM mipi_csi_vdd enable failed\n", __func__);
				return rc;
			}
		}
		#endif

		rc = msm_camera_config_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: regulator on failed\n", __func__);
			goto config_vreg_failed;
		}
		
		rc = msm_camera_enable_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: enable regulator failed\n", __func__);
			goto enable_vreg_failed;
		}
		
		#if defined (CONFIG_OV5640)||defined (CONFIG_OV7692)
		if (cam_vio == NULL) {
		cam_vio = regulator_get(dev, "cam_vio");
		if (IS_ERR(cam_vio)) {
			pr_err("%s: VREG CAM VIODD get failed\n", __func__);
		}
		if (regulator_enable(cam_vio)) {
				pr_err("%s: VREG CAM VIODD enable failed\n", __func__);
			}
		}
		#endif	
		hasvreged=1;
		return rc;
enable_vreg_failed:
	msm_camera_config_vreg(dev,
		sinfo->sensor_platform_info->cam_vreg,
		sinfo->sensor_platform_info->num_vreg,
		reg_ptr, 0);
config_vreg_failed:
	msm_camera_request_gpio_table(sinfo, 0);
	return rc;

		}else{
		pr_err("%s: %d\n", __func__, __LINE__);
		#if 0
		if (mipi_csi_vdd == NULL) {
		mipi_csi_vdd = regulator_get(dev, "mipi_csi_vdd");
		if (IS_ERR(mipi_csi_vdd)) {
			pr_err("%s: VREG CAM mipi_csi_vdd get failed\n", __func__);
			return rc;
		}
		rc = regulator_set_voltage(
				mipi_csi_vdd,
				1200000,
				1200000);
		rc = regulator_set_optimum_mode(
						mipi_csi_vdd,
						20000);
		if (regulator_enable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM mipi_csi_vdd enable failed\n", __func__);
				return rc;
			}
		}
		#endif


	return 0;
		}
		
}

#else
static bool hasvreged=0;
//static struct regulator *mipi_csi_vdd;
int msm_camera_power_on(struct device *dev,struct msm_camera_sensor_info *sinfo,struct regulator **reg_ptr)
{		
		int rc=0;
		if(hasvreged==0){
		
		pr_err("%s: %d\n", __func__, __LINE__);
		#if 0
		if (mipi_csi_vdd == NULL) {
		mipi_csi_vdd = regulator_get(dev, "mipi_csi_vdd");
		if (IS_ERR(mipi_csi_vdd)) {
			pr_err("%s: VREG CAM mipi_csi_vdd get failed\n", __func__);
			return rc;
		}
		rc = regulator_set_voltage(
				mipi_csi_vdd,
				1200000,
				1200000);
		rc = regulator_set_optimum_mode(
						mipi_csi_vdd,
						20000);
		if (regulator_enable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM mipi_csi_vdd enable failed\n", __func__);
				return rc;
			}
		}
		#endif

		rc = msm_camera_config_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: regulator on failed\n", __func__);
			goto config_vreg_failed;
		}
		
		rc = msm_camera_enable_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: enable regulator failed\n", __func__);
			goto enable_vreg_failed;
		}

	hasvreged=1;
	return rc;
}else{
		pr_err("%s: %d\n", __func__, __LINE__);
		#if 0
		if (mipi_csi_vdd == NULL) {
		mipi_csi_vdd = regulator_get(dev, "mipi_csi_vdd");
		if (IS_ERR(mipi_csi_vdd)) {
			pr_err("%s: VREG CAM mipi_csi_vdd get failed\n", __func__);
			return rc;
		}
		rc = regulator_set_voltage(
				mipi_csi_vdd,
				1200000,
				1200000);
		rc = regulator_set_optimum_mode(
						mipi_csi_vdd,
						20000);
		if (regulator_enable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM mipi_csi_vdd enable failed\n", __func__);
				return rc;
			}
		}
		#endif

	return 0;
}
enable_vreg_failed:
	msm_camera_config_vreg(dev,
		sinfo->sensor_platform_info->cam_vreg,
		sinfo->sensor_platform_info->num_vreg,
		reg_ptr, 0);
config_vreg_failed:
	msm_camera_request_gpio_table(sinfo, 0);
	return rc;

}
#endif
#if 0
int msm_camera_power_off(struct device *dev,struct msm_camera_sensor_info *sinfo,struct regulator **reg_ptr)
{		
		int rc=0;
		
		#if defined (CONFIG_OV5640) || defined (CONFIG_OV7692)
		if (regulator_disable(cam_vdig)) {
				pr_err("%s: VREG CAM VDIG enable failed\n", __func__);
			}
		regulator_put(cam_vdig);
		cam_vdig = NULL;
		#endif
		
		 rc= msm_camera_enable_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 0);
		rc= msm_camera_config_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 0);
				
	return rc;

	
}
#endif
#if 1
int msm_camera_power_off(struct device *dev,struct msm_camera_sensor_info *sinfo,struct regulator **reg_ptr)
{				
#ifdef CONFIG_MACH_KISKA
		 int rc=0;
		pr_err("%s: power off entered ygl\n", __func__);
		 rc= msm_camera_enable_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 0);
		if (rc < 0) {
			pr_err("%s: power off failed\n", __func__);
			return 0;
		}
		rc= msm_camera_config_vreg(dev,
				sinfo->sensor_platform_info->cam_vreg,
				sinfo->sensor_platform_info->num_vreg,
				reg_ptr, 0);
		if (rc < 0) {
			pr_err("%s: power off failed\n", __func__);
			return 0;
		}


		rc = gpio_request(89,"SENSOR_NAME");
		if (!rc) {
			pr_err("%s: gpio_request 89 OK_1\n", __func__);
			gpio_direction_output(89,0);
			usleep_range(1000, 2000);
			mdelay(1);
		} else {
			gpio_free(89);
			mdelay(1);
			rc = gpio_request(89,"SENSOR_NAME");
			if (!rc){
			pr_err("%s: gpio_request 89 OK_2\n", __func__);
			gpio_direction_output(89,0);
			usleep_range(1000, 2000);
			mdelay(1);
			}else{
			pr_err("%s: gpio_request 89 failed\n", __func__);
			}
		}

		rc = gpio_request(9,"SENSOR_NAME");
		if (!rc) {
			pr_err("%s: gpio_request 09 OK_1\n", __func__);
			gpio_direction_output(9,0);
			usleep_range(1000, 2000);
			mdelay(1);
		} else {
			gpio_free(9);
			mdelay(1);
			rc = gpio_request(9,"SENSOR_NAME");
			if (!rc){
			pr_err("%s: gpio_request 09 OK_2\n", __func__);
			gpio_direction_output(9,0);
			usleep_range(1000, 2000);
			mdelay(1);
			}else{
			pr_err("%s: gpio_request 09 failed\n", __func__);
			}
		}
		
#endif
		return 0;
}

#else//power off mipi
int msm_camera_power_off(struct device *dev,struct msm_camera_sensor_info *sinfo,struct regulator **reg_ptr)
{		
		
		if (regulator_disable(mipi_csi_vdd)) {
				pr_err("%s: VREG CAM VDIG enable failed\n", __func__);
			}
		regulator_set_optimum_mode(mipi_csi_vdd, 0);
		regulator_set_voltage(mipi_csi_vdd,0, 1200000);
		regulator_put(mipi_csi_vdd);
		mipi_csi_vdd = NULL;
		
		return 0;

}
#endif
