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
#include <linux/file.h>
#include <linux/fs.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <linux/slab.h> 
#include <linux/vmalloc.h> 

#include <linux/android_pmem.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/vt_kern.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

#include <linux/irq.h>
#include <asm/system.h>

#include <linux/regulator/consumer.h>
#include <mach/vreg.h>

#include "isp_cam.h"

static int isp_boot_flags=0;
static struct regulator *ldo116;

extern int flash_stats;

#define SENSOR_NAME "ispcam"
#define PLATFORM_DRIVER_NAME "msm_camera_ispcam"
#define isp_cam_obj isp_cam_##obj

#define ISP_CAM_FW_WRITER_MODE 1
#define ISP_CAM_NORMAL_BOOT 1

#define M7MO_CUSTOM_DEFAULT_POLL_TIME		20 		// 20 ms
#define M7MO_CUSTOM_DEFAULT_POLL_COUNT		500		// 300 times

#define	CMD_ERROR_COMM_NUM_ERROR		0xF0
#define	CMD_ERROR_CMD_ERROR				0xF1
#define	CMD_ERROR_CATE_ERROR			0xF2
#define	CMD_ERROR_BYTE_ERROR			0xF3
#define	CMD_ERROR_BYTE_LEN_ERROR		0xF4
#define	CMD_ERROR_M7MO_BUSY				0xFA
#define	CMD_ERROR_MODE_ERROR			0xFF

static int isp_cam_state=1;

int touchafaex=0;
int touchafaey=0;
int touchdafaedx=0;
int touchafae_state=0;

int caf_state=0;

int mult_cap_statue=0;

#define M7MO_Custom_Delay(n) mdelay(n)
int32_t msm_isp_cam_fwup_init(struct i2c_client *client,
	const struct i2c_device_id *id);

int32_t msm_isp_cam_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id);

static int isp_fw_wr_flags=0;

/*
static int get_ispfw_w_flag()
{
    char value[PROPERTY_VALUE_MAX];
    mCameraOpen = false;
    property_get("persist.camera.hal.multitouchaf", value, "0");
    return atoi(value);
}  
  */

static int gpio_isp_spi_arr[]={93,95,96};

static unsigned isp_spi_config_off[] = {
	GPIO_CFG(93, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/*  isp spi data */ 
    GPIO_CFG(95, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/*  isp spi siocs */
	GPIO_CFG(96, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),	/*  isp spi siock */
};


static void disable_isp_spi(void)
{
	int pin = 0, rc = 0;

	rc = gpio_request(46,"ispcam");
		if (!rc) {
			pr_err("%s: gpio_request 46 OK\n", __func__);
			gpio_direction_output(46,1);
			usleep_range(1000, 2000);
			mdelay(1);
		} else {
			gpio_free(46);
			mdelay(1);
			rc = gpio_request(46,"ispcam");
			if (!rc){
			gpio_direction_output(46,1);
			usleep_range(1000, 2000);
			mdelay(1);
			}else{
			pr_err("%s: gpio_request 46 failed\n", __func__);
			}
		}
	
	pr_err("%s isp spi config off ",__func__);
	for (pin = 0; pin < ARRAY_SIZE(isp_spi_config_off); pin++) 
	{
	    rc =gpio_request(gpio_isp_spi_arr[pin],"isp_spi");
	   
		rc = gpio_tlmm_config(isp_spi_config_off[pin],GPIO_CFG_ENABLE);
		if (rc) 
		{
			printk(KERN_ERR
			       " error : gpio_tlmm_config(%#x)=%d\n",
			        isp_spi_config_off[pin], rc);
		}
		else
		{
         pr_err("%s: gpio_tlmm_config(%#x)=%d\n", __func__, isp_spi_config_off[pin], rc);
		}

		rc = gpio_direction_input(gpio_isp_spi_arr[pin]);
        if (rc) {
        printk(KERN_ERR "%s: gpio_direction_input(%#x)=%d\n",
                __func__,gpio_isp_spi_arr[pin] , rc);
		
        
    	}
	}



    }

static int ispcam_vreg_disable(void)
{
	int rc=0;
	pr_err("%s:  %d\n", __func__, __LINE__);

    //motor
	ldo116 = regulator_get(NULL, "8921_l16");
	if (IS_ERR(ldo116)) {
		pr_err("%s: VREG ldo116 get failed\n", __func__);
		ldo116 = NULL;
		return -1;
	}
	//regulator_set_mode(ldo116,REGULATOR_MODE_IDLE);
	regulator_disable(ldo116);




	
	

	return rc;
		
}
static void  ispcam_vreg_enable(void)
{
		//(MOTOR_2V85
		
		ldo116 = regulator_get(NULL, "8921_l16");
		pr_err("%s",__func__);
		if (IS_ERR(ldo116)) {
			pr_err("%s: VREG LDO16 get failed\n", __func__);
			ldo116 = NULL;
			goto ldo116_disable;
			
		}
		if (regulator_set_voltage(ldo116, 2800000, 2800000)) {
			pr_err("%s: VREG LDO16 set voltage failed\n",  __func__);
			goto ldo116_disable;
		}
		
		//regulator_set_mode(ldo116,REGULATOR_MODE_NORMAL);
		if (regulator_enable(ldo116)) {
			pr_err("%s: VREG LDO16 enable failed\n", __func__);
			goto ldo116_put;
		}



ldo116_disable:
	regulator_disable(ldo116);
ldo116_put:
	regulator_put(ldo116);


	

}
DEFINE_MUTEX(isp_cam_mut);




static struct msm_sensor_ctrl_t *ispcam_s_ctrl;

static struct msm_sensor_ctrl_t isp_cam_s_ctrl;

static struct i2c_client *isp_i2c_client;
const static struct i2c_device_id *isp_id;

static int isp_ini_flag=0;

//xuyao
static struct msm_camera_i2c_reg_conf isp_cam_start_settings[] = {
	//{0x0100, 0x01},
};

//xuyao
static struct msm_camera_i2c_reg_conf isp_cam_stop_settings[] = {
	//{0x0100, 0x00},
};

//xuyao

static struct msm_camera_i2c_reg_conf isp_cam_groupon_settings[] = {
	//{0x3208, 0x00},
};
//xuyao

static struct msm_camera_i2c_reg_conf isp_cam_groupoff_settings[] = {
	//{0x3208, 0x10},
	//{0x3208, 0xA0},
};
//xuyao

static struct msm_camera_i2c_reg_conf isp_cam_prev_settings[] = {
	//{0x3005, 0x11},
	
};
//xuyao

static struct msm_camera_i2c_reg_conf isp_cam_snap_settings[] = {
	//{0x3005, 0x10},
	
};
//xuyao

static struct msm_camera_i2c_reg_conf isp_cam_reset_settings[] = {
	//{0x0103, 0x01},
};
//xuyao

static struct msm_camera_i2c_reg_conf isp_cam_recommend_settings[] = {
	//{0x0103, 0x01},
};

static struct v4l2_subdev_info isp_cam_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array isp_cam_init_conf[] = {
	{&isp_cam_reset_settings[0],
	ARRAY_SIZE(isp_cam_reset_settings), 50, MSM_CAMERA_I2C_BYTE_DATA},
	{&isp_cam_recommend_settings[0],
	ARRAY_SIZE(isp_cam_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array isp_cam_confs[] = {
	{&isp_cam_snap_settings[0],
	ARRAY_SIZE(isp_cam_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&isp_cam_prev_settings[0],
	ARRAY_SIZE(isp_cam_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t isp_cam_dimensions[] = {
#if 0

	{
		.x_output = 2592,//2592,
		.y_output = 1944,
		.line_length_pclk = 2592,
		.frame_length_lines = 1944,
		.vt_pixel_clk = 216000000,//162000000,//216000000
		.op_pixel_clk = 216000000,//162000000,
		.binning_factor = 1,
	},
	{

		.x_output = 640, //0x140,//0x500,
		.y_output = 480,//0x2D0,
		.line_length_pclk = 640,
		.frame_length_lines = 480,
		.vt_pixel_clk = 162000000,//162000000, // 216000000
		.op_pixel_clk = 162000000,//162000000, //vfe
		.binning_factor = 1,
   #else
      /* 5m {
        .x_output = 2592,//0x280,
        .y_output =  1944,//0x1E0,
        .line_length_pclk = 2592,//0x280,
        .frame_length_lines =1944,//0x1E0,
        .vt_pixel_clk = 216000000,//162000000,
        .op_pixel_clk = 216000000,//162000000,
        .binning_factor = 1,
  
	},*/
	{
        .x_output = 4000,//0x280,
        .y_output =  3000,//0x1E0,
        .line_length_pclk = 4000,//0x280,
        .frame_length_lines =3000,//0x1E0,
        .vt_pixel_clk = 216000000,//162000000,
        .op_pixel_clk = 216000000,//162000000,
        .binning_factor = 1,
  
	},
   {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 1920,
        .frame_length_lines = 1080,
        .vt_pixel_clk = 165888000,
        .op_pixel_clk = 165888000,
        .binning_factor = 1,
  
	},
	 #endif
};

static struct msm_camera_csid_vc_cfg isp_cam_cid_cfg[] = {
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params isp_cam_cap_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = isp_cam_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x14,//0x14 wt 26
	},
};

static struct msm_camera_csi2_params isp_cam_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
		.lane_cnt = 2,
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = isp_cam_cid_cfg,
		},
	},
	.csiphy_params = {
		.lane_cnt = 2,
		.settle_cnt = 0x14,//0x14 wt
	},
};




static struct msm_camera_csi2_params *isp_cam_csi_params_array[] = {
	&isp_cam_cap_csi_params,
	&isp_cam_csi_params,
};

static struct msm_sensor_output_reg_addr_t isp_cam_reg_addr = {
	.x_output = 0x3808,
	.y_output = 0x380a,
	.line_length_pclk = 0x380c,
	.frame_length_lines = 0x380e,
};

static struct msm_sensor_id_info_t isp_cam_id_info = {
	.sensor_id_reg_addr = 0x300A,
	.sensor_id = 0x8820,
};

static struct msm_sensor_exp_gain_info_t isp_cam_exp_gain_info = {
	.coarse_int_time_addr = 0x3501,
	.global_gain_addr = 0x350A,
	.vert_offset = 6,
};



//
// Library Functions
//

// Context : User
// Parameter :
//   filename : filename to open
//   flags :
//     O_RDONLY, O_WRONLY, O_RDWR
//     O_CREAT, O_EXCL, O_TRUNC, O_APPEND, O_NONBLOCK, O_SYNC, ...
//   mode : file creation permission.
//     S_IRxxx S_IWxxx S_IXxxx (xxx = USR, GRP, OTH), S_IRWXx (x = U, G, O)
// Return :
//   file pointer. if error, return NULL

struct file *klib_fopen(const char *filename, int flags, int mode)
{
	struct file* pfile = NULL;
	struct inode *inode;
	off_t fsize=0;

    pfile = filp_open(filename, O_RDONLY, 0);

	if(IS_ERR(pfile))
	{
     pr_err("klib_fopen  %s is error ! \n",filename);


	 return NULL;
	}
	else
	{
		pr_err("klib_fopen %s  is ok \n",filename);
		inode=pfile->f_dentry->d_inode; 
 	    fsize=inode->i_size; 
		pr_err("fsize =%ld \n",fsize);
	
     return  pfile;
	}

	return NULL;
	
}

//#ifdef ISP_CAM_FW_WRITER_MODE
#if 1

// Context : User
// Parameter :
//   filp : file pointer
// Return :

void klib_fclose(struct file *filp)
{
    if (filp)
        fput(filp);
}

// Context : User
// Parameter :
//   filp : file pointer
//   offset :
//   whence : SEEK_SET, SEEK_CUR
// Comment :
//   do not support SEEK_END
//   no boundary check (file position may exceed file size)

int klib_fseek(struct file *filp, int offset, int whence)
{
        int pos = filp->f_pos;

    if (filp) {
                if (whence == SEEK_SET)
                        pos = offset;
                else if (whence == SEEK_CUR)
                        pos += offset;
                if (pos < 0)
                        pos = 0;
        return (filp->f_pos = pos);
    } else
        return -ENOENT;
}
// Context : User
// Parameter :
//   buf : buffer to read into
//   len : number of bytes to read
//   filp : file pointer
// Return :
//   actually read number. 0 = EOF, negative = error

int klib_fread(char *buf, int len, struct file *filp)
{
    int readlen;
    mm_segment_t oldfs;

    if (filp == NULL)
                return -ENOENT;
        if (filp->f_op->read == NULL)
                return -ENOSYS;
        if (((filp->f_flags & O_ACCMODE) & O_RDONLY) != 0)
                return -EACCES;

        oldfs = get_fs();
        set_fs(KERNEL_DS);
        readlen = filp->f_op->read(filp, buf, len, &filp->f_pos);
        set_fs(oldfs);

    return readlen;
}
// Context : User
// Parameter :
//   filp : file pointer
// Return :
//   read character, EOF if end of file

int klib_fgetc(struct file *filp)
{
        int len;
        unsigned char buf[4];

        len = klib_fread((char *) buf, 1, filp);

        if (len > 0)
                return buf[0];
        else if (len == 0)
                return EOF;
        else
                return len;
}

// Context : User
// Parameter :
//   str : string
//   size : size of str buffer
//   filp : file pointer
// Return :
//   read string. NULL if end of file
// Comment :
//   Always append trailing null character
char *klib_fgets(char *str, int size, struct file *filp)
{
        char *cp;
    int len, readlen;
    mm_segment_t oldfs;

    if (filp && filp->f_op->read && ((filp->f_flags &O_ACCMODE) & O_WRONLY) == 0) {
        oldfs = get_fs();
        set_fs(KERNEL_DS);

                for (cp = str, len = -1, readlen = 0; readlen < size - 1; ++cp,++readlen) {
                if ((len = filp->f_op->read(filp, cp, 1, &filp->f_pos))<= 0)
                                break;
                        if (*cp == '\n') {
                                ++cp;
                                ++readlen;
                                break;
                        }
                }

                *cp = 0;

                set_fs(oldfs);

            return (len < 0 || readlen == 0) ? NULL : str;
    } else
                return NULL;
}

// Context : User
// Parameter :
//   buf : buffer containing data to write
//   len : number of bytes to write
//   filp : file pointer
// Return :
//   actually written number. 0 = retry, negative = error

int klib_fwrite(char *buf, int len, struct file *filp)
{
    int writelen;
    mm_segment_t oldfs;

    if (filp == NULL)
                return -ENOENT;
        if (filp->f_op->write == NULL)
                return -ENOSYS;
        if (((filp->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR)) == 0)
                return -EACCES;

        oldfs = get_fs();
        set_fs(KERNEL_DS);
        writelen = filp->f_op->write(filp, buf, len, &filp->f_pos);
        set_fs(oldfs);

    return writelen;
}

// Context : User
// Parameter :
//   filp : file pointer
// Return :
//   written character, EOF if error

int klib_fputc(int ch, struct file *filp)
{
        int len;
        unsigned char buf[4];

        buf[0] = (unsigned char) ch;
        len = klib_fwrite(buf, 1, filp);

        if (len > 0)
                return buf[0];
        else
                return EOF;
}
// Context : User
// Parameter :
//   str : string
//   filp : file pointer
// Return :
//   count of written characters. 0 = retry, negative = error

int klib_fputs(char *str, struct file *filp)
{
        return klib_fwrite(str, strlen(str), filp);
}

// Context : User
// Parameter :
//   filp : file pointer
//   fmt : printf() style formatting string
// Return :
//   same as klib_fputs()

int klib_fprintf(struct file *filp, const char *fmt, ...)
{
        static char s_buf[1024];

        va_list args;

        va_start(args, fmt);
        vsprintf(s_buf, fmt, args);
        va_end(args);

        return klib_fputs(s_buf, filp);
}
#endif

 int test_isp_cam_i2c_rxdata(unsigned short saddr,
                                   unsigned char *txdata,  
                                   unsigned char *rxdata,
                                   int length)
{
    struct i2c_msg msgs[] = {
        {
            .addr  = saddr,
            .flags = 0,
            .len   = 5,
            .buf   = txdata,
        },
        {
            .addr  = saddr,
            .flags = I2C_M_RD,
            .len   = length,
            .buf   = rxdata,
        },
    };
	
    if (i2c_transfer(isp_i2c_client->adapter, msgs, 2) < 0)
    {
        pr_err("%s: failed!\n", __func__);
        return -EIO;
    }
	//pr_err("%s: ok !\n", __func__);

    return 0;
}




/**
 *	@brief		Write one byte to specified category and byte
 *  @param[in]	cat			Indicate which category to be written
 *  @param[in]	byte		Indicate which byte of the category to be written
 *  @param[in]	data		value to be written
 *	@return		None
 *	@note		None
 *	@attention
*/
int M7MO_WriteOneByteCRAM(unsigned char cat,unsigned char byte,unsigned char data)
{
    int rc = -EFAULT;
	unsigned char cmd_senddata[8];
	cmd_senddata[0] = 5;
	cmd_senddata[1] = 2;
	cmd_senddata[2] = cat;
	cmd_senddata[3] = byte;
	cmd_senddata[4] = data;

    pr_err("%s: cat = 0x%02x, byte = 0x%02x, data = 0x%02x !\n", __func__, cat, byte, data);
	rc = isp_cam_i2c_txdata(ispcam_s_ctrl,0x1f, cmd_senddata, 5);

    if (rc < 0)
    {
        pr_err("%s: cat = 0x%x, byte = 0x%x, data = 0x%x, failed!\n", __func__, cat, byte, data);
    }
    return rc;
}



/**
 *	@brief		Read one byte from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		The byte required to be read
 *	@note		None
 *	@attention
*/
unsigned char M7MO_ReadOneByteCRAM(unsigned char cat,unsigned char byte)
{
    int rc = -EFAULT;
	unsigned char cmd_senddata[8];
	unsigned char cmd_recdata[8];
	cmd_senddata[0] = 5;
	cmd_senddata[1] = 1;
	cmd_senddata[2] = cat;
	cmd_senddata[3] = byte;
	cmd_senddata[4] = 1;
	//Read 2 byte: 1 byte for length and 1 byte data 
	rc = test_isp_cam_i2c_rxdata(0x1f, cmd_senddata, cmd_recdata, 2);
    if (rc < 0)
    {
        pr_err("%s: cat = 0x%x, byte = 0x%x, failed!\n", __func__, cat, byte);
    }

	pr_err("cmd_recdata[1]=%d \n",cmd_recdata[1]);
	return cmd_recdata[1];
}




/**
 *	@brief		Write two byte to specified category and byte
 *  @param[in]	cat			Indicate which category to be written
 *  @param[in]	byte		Indicate the starting byte of the category to be written
 *  @param[in]	data		value to be written
 *	@return		None
 *	@note		None
 *	@attention
*/
int M7MO_WriteOneHalfwordCRAM( unsigned char cat, unsigned char byte, unsigned short data )
{
    int rc = -EFAULT;
	unsigned char cmd_senddata[8];
	cmd_senddata[0] = 6;
	cmd_senddata[1] = 2;
	cmd_senddata[2] = cat;
	cmd_senddata[3] = byte;
	cmd_senddata[4] = (unsigned char)( data >>8 );
	cmd_senddata[5] = (unsigned char)( data &0x00FF );

   // pr_err("%s: cat = 0x%x, byte = 0x%x, data = 0x%x !\n", __func__, cat, byte, data);
	rc = isp_cam_i2c_txdata(ispcam_s_ctrl,0x1f, cmd_senddata, 6);
    if (rc < 0)
    {
        pr_err("%s: cat = 0x%x, byte = 0x%x, data = 0x%x, failed!\n", __func__, cat, byte, data);
    }
    return rc;
    
}



/**
 *	@brief		Read two byte from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		The byte required to be read
 *	@note		None
 *	@attention
*/
unsigned short M7MO_ReadOneHalfwordCRAM( unsigned char cat, unsigned char byte )
{
    int rc = -EFAULT;
	unsigned char	cmd_senddata[8];
	unsigned char	cmd_recdata[8];
	unsigned short	ret;
	cmd_senddata[0] = 5;
	cmd_senddata[1] = 1;
	cmd_senddata[2] = cat;
	cmd_senddata[3] = byte;
	cmd_senddata[4] = 2;
	//Read 3 byte: 1 byte for length and 2 byte data
	rc = isp_cam_i2c_rxdata(ispcam_s_ctrl, 0x1f,cmd_senddata, cmd_recdata, 3);
    if (rc < 0)
    {
        pr_err("%s: cat = 0x%x, byte = 0x%x, failed!\n", __func__, cat, byte);
    }	

	ret  = cmd_recdata[1]<<8;
	ret += cmd_recdata[2];
	return	ret;
}




/**
 *	@brief		Write four byte to specified category and byte
 *  @param[in]	cat			Indicate which category to be written
 *  @param[in]	byte		Indicate the starting byte of the category to be written
 *  @param[in]	data		value to be written
 *	@return		None
 *	@note		None
 *	@attention
*/
int M7MO_WriteOneWordCRAM( unsigned char cat, unsigned char byte, unsigned long data )
{
    int rc = -EFAULT;
	unsigned char cmd_senddata[8];
	cmd_senddata[0] = 8;
	cmd_senddata[1] = 2;
	cmd_senddata[2] = cat;
	cmd_senddata[3] = byte;
	cmd_senddata[4] = (unsigned char)(( data &0xFF000000 )>>24 );
	cmd_senddata[5] = (unsigned char)(( data &0x00FF0000 )>>16 );
	cmd_senddata[6] = (unsigned char)(( data &0x0000FF00 )>>8 );
	cmd_senddata[7] = (unsigned char)(  data &0x000000FF );
        //pr_err("%s: cat = 0x%x, byte = 0x%x, data = 0x%ld!\n", __func__, cat, byte, data);
    
	rc = isp_cam_i2c_txdata(ispcam_s_ctrl,0x1f, cmd_senddata, 8);
    if (rc < 0)
    {
        pr_err("%s: cat = 0x%x, byte = 0x%x, data = 0x%ld failed!\n", __func__, cat, byte, data);
    }
    
	return rc;

    
}



/**
 *	@brief		Read four byte from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		The byte required to be read
 *	@note		None
 *	@attention
*/
unsigned long M7MO_ReadOneWordCRAM( unsigned char cat, unsigned char byte )
{
    int rc = -EFAULT;
	unsigned char	cmd_senddata[8];
	unsigned char	cmd_recdata[8] ={0,0,0,0,0,0,0,0};
	unsigned long	ret;
	cmd_senddata[0] = 5;
	cmd_senddata[1] = 1;
	cmd_senddata[2] = cat;
	cmd_senddata[3] = byte;
	cmd_senddata[4] = 4;
	//Read 5 byte: 1 byte for length and 4 byte data
	rc = isp_cam_i2c_rxdata(ispcam_s_ctrl, 0x1f,cmd_senddata, cmd_recdata, 5);
    if (rc < 0)
    {
        pr_err("%s: cat = 0x%x, byte = 0x%x, failed!\n", __func__, cat, byte);
    }	

	ret  = cmd_recdata[1]<<24;
	ret += cmd_recdata[2]<<16;
	ret += cmd_recdata[3]<<8;
	ret += cmd_recdata[4];
	return	ret;
}


/**
 *	@brief		Read bytes from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		pointer to the received data
 *	@note		None
 *	@attention
*/
void M7MO_ReadBytesCRAM( unsigned char cat, unsigned char byte , unsigned char byteCnt, unsigned char* cmd_recdata)
{
    int rc = -EFAULT;
	unsigned char	cmd_senddata[8];
	cmd_senddata[0] = 5;
	cmd_senddata[1] = 1;
	cmd_senddata[2] = cat;
	cmd_senddata[3] = byte;
	cmd_senddata[4] = byteCnt;
	//Read byteCnt+1 byte: 1 byte for length and byteCnt byte data
	rc = isp_cam_i2c_rxdata(ispcam_s_ctrl,0x1f, cmd_senddata,  cmd_recdata, byteCnt+1);
    if (rc < 0)
    {
        pr_err("%s: cat = 0x%x, byte = 0x%x, failed!\n", __func__, cat, byte);
    }		
    
}


int isp_camera_flash_led_disable(void)
{
   int rc=0;
   /*
   	if(isp_cam_state_preview)
	{
	   M7MO_WriteOneByteCRAM( 0x03, 0x35, 0x00 );
	   pr_err("flash  isp_camera_flash_led_disable");
   	}
   else
   {
	   pr_err("flash  snapshot ,cannt set flash");
   }
   */

	return rc;
}

int isp_camera_flash_led_enable(void)
{
	int rc=0;
	/*
	if(isp_cam_state_preview)
	{
	M7MO_WriteOneByteCRAM( 0x03, 0x35, 0x01 );
	pr_err("flash  isp_camera_flash_led_enable");
	}
	else
	{
		pr_err("flash  snapshot ,cannt set flash");
	}
	*/

	return rc;

}

int isp_camera_flash_led_auto(void)
{
	int rc=0;
	/*
	if(isp_cam_state_preview)
	{
		M7MO_WriteOneByteCRAM( 0x03, 0x35, 0x02 );
		pr_err("flash  isp_camera_flash_led_auto");
	}
	else
	{
     	pr_err("flash  snapshot ,cannt set flash");
	}*/
	return rc;

}


/**
 *	@brief		M7MO memory write (8 bit access)
 *  @param[in]	send_buf		address of send buffer, the data to be written at the specified 
 *  							address should be stored at the (send_buf + M7MO_WRITEMEMORY_START) 
 *  							position. It is because the first 8 bytes are to be reserved for 
 *  							the I2C command header.
 *  @param[in]	buf_size		the size of the send_buf
 *  @param[in]	addr			the destination address to be written in M7MO
 *  @param[in]	write_size		size of data to be written
 *	@return		0: OK
 *	@return		1: NG
 *	@note		None
 *	@par Example use:
 *	@code
 *	unsigned char send_buf[16];
 *	int value = 1;
 *	memcpy(send_buf + M7MO_WRITEMEMORY_START, &value, sizeof(int)); // the first 8 bytes should be reserved!
 *	M7MO_WriteMemory(send_buf, sizeof(send_buf), 0x80000000, sizeof(int));
 *	@endcode
 */
int M7MO_WriteMemory(unsigned char * send_buf, unsigned long buf_size, unsigned long addr, unsigned long write_size)
{
	if(buf_size >= 8 + write_size)
	{
		int ret;
		send_buf[0] = 0x0;
		send_buf[1] = 4;	//CMD_EX_WRITE
		send_buf[2] = (unsigned char)((addr & 0xFF000000) >> 24);
		send_buf[3] = (unsigned char)((addr & 0x00FF0000) >> 16);
		send_buf[4] = (unsigned char)((addr & 0x0000FF00) >>  8);
		send_buf[5] = (unsigned char)( addr & 0x000000FF);
		send_buf[6] = (unsigned char)((write_size & 0xFF00) >> 8);
		send_buf[7] = (unsigned char)(write_size & 0x00FF);		
		ret = isp_cam_i2c_txdata(ispcam_s_ctrl,0x1f, send_buf, 8 + write_size);
		return ret;
	}else{
		// send_buf's size not large enough
		// note that send_buf's first 8 bytes should be reserved for I2C command header
		return -1;
	}
}



/**
 * Wait until specified interrupt occured by polling with timeout.
 * @param[in] factor	a bitwise OR of the interrupt factors to wait
 * @param[in] mode 		the waiting mode
 * 						M7MO_WAIT_AND: wait until all factors are received
 * 						M7MO_WAIT_OR: wait until any of the factors are received
 * 						M7MO_WAIT_PEEK: Check if the specified interrupt are received and return immediately
 * @return 	0 when no factor occured before timeout; otherwise, it returns
 * 			the bitwise OR of the interrupt factors received
 */
	int M7MO_Custom_PollInterrupt(int factor, int mode)
{
		int loop;
		int curFactor = 0;
		int gotFactor = 0;
		unsigned int iteration;
		if(mode == M7MO_WAIT_PEEK)
			iteration = 1;
		else
			iteration = M7MO_CUSTOM_DEFAULT_POLL_COUNT;
	
		for( loop = 0; factor && loop < iteration; loop++ )
		{
			// delay
			M7MO_Custom_Delay(M7MO_CUSTOM_DEFAULT_POLL_TIME);
			// Read interrupt factor
			curFactor = (int)M7MO_ReadOneByteCRAM( 0x00, 0x1C );
	        pr_err("loop=%d curFactor=0x%x\n",loop,curFactor);
			if(curFactor == CMD_ERROR_COMM_NUM_ERROR ||
					curFactor == CMD_ERROR_CMD_ERROR ||
					curFactor == CMD_ERROR_CATE_ERROR ||
					curFactor == CMD_ERROR_BYTE_ERROR ||
					curFactor == CMD_ERROR_BYTE_LEN_ERROR ||
					curFactor == CMD_ERROR_MODE_ERROR ||
					curFactor == CMD_ERROR_M7MO_BUSY )
			{

				pr_err("M7MO_Custom_PollInterrupt Error: %08x\n", factor);

			}
			else if(curFactor & factor){
				gotFactor |= (curFactor & factor);
				// clear obtained factor(s)
				factor &= ~curFactor;
				if(mode == M7MO_WAIT_OR){
					break;
				}
			}
		}

		pr_err("M7MO_Custom_PollInterrupt Ack: %08x\n", gotFactor);

		return gotFactor;
	}



//M7MO_Function\M7MO_IntHandler.c

/**
 *	@brief		Function to wait interrupt generated from M7MO
 *  @param[in]	factor 		interrupt factor to wait
 *  @param[in]	mode		waiting mode
 *  						M7MO_WAIT_AND: wait until all specified factors are interrupted
 *  						M7MO_WAIT_OR: wait until any one of the specified factors are interrupted
  * 						M7MO_WAIT_PEEK: Check if the specified interrupt are received and return immediately
 *	@return		interrupt factor byte that indicate which kind of interrupt has occured
 *	@note		None
 *	@attention
 */
int M7MO_WaitInterrupt(int factor, int mode)
{
	int ret;
	//Funciton to be implemented by host to wait M7MO interrupt
	ret = M7MO_Custom_PollInterrupt(factor, mode);
	return ret;
}




int M7MO_System_Init(void)
{
	int i, ret;
	//Write: CAM_START = '0x01' (i.e. write category:0x0F, byte: 0x12 to 0x01)
	M7MO_WriteOneByteCRAM(0x0F, 0x12, 0x01); 
	M7MO_Custom_Delay(50);
	//Read system mode until a normal value instead of 0xFF is returned 
	for( i=0 ; i<2500; i++ ){
		M7MO_Custom_Delay(10);
		ret = M7MO_ReadOneByteCRAM(0x00, 0x0B);
		if(( ret & 0xF0 )!= 0xF0 ){
           pr_err(" isp M7MO_System_Init M7MO_OK \n");

		   M7MO_WaitInterrupt(M7MO_HOSTINT_MON, M7MO_WAIT_AND); //Thomas
		   
		   return M7MO_OK;
		}
	}
	pr_err(" isp M7MO_System_Init M7MO_NG \n");
	return M7MO_NG;
}


int M7MO_VER(void)
{
   int isp_ver_h=0;
   int isp_ver_l=0;
   int isp_version=0;

   pr_err("M7MO_VER \n");

    isp_ver_h=M7MO_ReadOneByteCRAM(0x00, 0x01);
	pr_err("0x00, 0x01 = 0x%02x \n",isp_ver_h);
	isp_ver_h=M7MO_ReadOneByteCRAM(0x00, 0x02);
	pr_err("0x00, 0x02 = 0x%02x \n",isp_ver_h);
	isp_ver_l=M7MO_ReadOneByteCRAM(0x00, 0x03);
	pr_err("0x00, 0x03 = 0x%02x \n",isp_ver_l);
	isp_ver_h=M7MO_ReadOneByteCRAM(0x00, 0x04);
	pr_err("0x00, 0x04 = 0x%02x \n",isp_ver_h);
	isp_ver_l=M7MO_ReadOneByteCRAM(0x00, 0x05);
	pr_err("0x00, 0x05 =0x%02x \n",isp_ver_l);

	isp_version=(isp_ver_h<<8)|(isp_ver_l);

	pr_err("M7MO_VER = 0x%02x \n",isp_version);

	return 0;


}

void HostCustom_Delay(unsigned int time)
{
	msleep(time);
}

void HostCustom_GetPreviewInit(void)
{
}

void HostCustom_GetPreviewCore(void)
{
}

void HostCustom_ShowPreview(void)
{
}

void HostCustom_GetThumbInit(void)
{
}

void HostCustom_GetThumbCore(void)
{
}

void HostCustom_SaveThumb(unsigned int thumbSize)
{
}

void HostCustom_GetMainInit(void)
{
}

void HostCustom_GetMainCore(void)
{
}

void HostCustom_SaveMain(unsigned int mainSize)
{
}

unsigned char HostCustom_IsSaveImage(void)
{
	return 1;
}

void HostCustom_SetThumbnailSize(unsigned int thumbSize)
{
}

void HostCustom_SetMainJpegSize(unsigned int mainSize)
{
}

void HostCustom_LeaveCapMode(void)
{
}





//TODO: load firmware form a file and reflash it
//This function is not completed now.
#ifdef ISP_CAM_FW_WRITER_MODE    



/**
 *	@brief		Write the data to flash rom
 *  @param[in]	send_buf		the address that stored the F/W on baseband
 *  @param[in]	send_buf_size	the size of the send buffer
 *	@return		None
 *	@note		The send_buf will become INVALID after calling the following function
 * 				since it will be overridden by I2C WriteMemory command headers
 */

static void M7MO_FwWriteMain( unsigned long send_buf, unsigned long send_buf_size)
{
  int	ret, loop;
	unsigned long	flash_addr, flash_end, write_size, read_addr, write_unit;
	unsigned short	w_unit;

	//¢Ú	The first address of FlashROM is set for chip erase
	M7MO_WriteOneWordCRAM( 0x0F, 0x00, 0x10000000 );
	pr_err("Start Chip Erase\r\n");

	//¢Û	The Chip erase command is issued
	M7MO_WriteOneByteCRAM( 0x0F, 0x06, 0x02 );

	//polling the completion of Chip erase
	for( loop = 0 ; loop < 1000 ; loop++ ){
		pr_err(".");
		M7MO_Custom_Delay( 50 );
		ret = M7MO_ReadOneByteCRAM( 0x0F, 0x06 );
		if( ret == 0x00 ){
			loop = 0;	// OK
			break;
		}
	}
	if( loop != 0 ){
		// Timeout (50ms*100=50s)
		pr_err("Chip Erase time out!\r\n");
		return ;
	}
	else{
		// Sector erase complete
		pr_err("Chip Erase complete!\r\n");
	}

	pr_err("Start Programming\n");

	flash_addr = WRITE_BUF_ADDR;
	flash_end  = WRITE_BUF_ADDR + WRITE_BUF_SIZE -1;
	write_size = WRITE_BUF_SIZE;
	write_unit = 0x8000;
	read_addr  = send_buf;

	// The loop for writing flash rom
	while( flash_addr < flash_end ){
		//¢Ü	The address of FlashROM is set for FrashROM writing. Please set the updated Sector Address, from the second round or later.
		M7MO_WriteOneWordCRAM( 0x0F, 0x00, flash_addr );

		//¢Ý	The size written in once is set to 64Kbyte.
		if(( write_size < write_unit )&&( write_size != 0 )){
			write_unit = write_size;
		}
		w_unit = (unsigned short)( write_unit &0x0000FFFF );
		M7MO_WriteOneHalfwordCRAM( 0x0F, 0x04, w_unit );

		//¢Þ	RAM for the buffer of M-7MO is cleared with 0xFF,
		M7MO_WriteOneByteCRAM( 0x0F, 0x08, 0x01 );

		//¢Þpolling the completion of clearing.
		for( loop = 0 ; loop < 100 ; loop++ ){
			M7MO_DEBUG_PRINT1(".");
			M7MO_Custom_Delay( 10 );
			ret = M7MO_ReadOneByteCRAM( 0x0F, 0x08 );
			if( ret == 0x00 ){
				loop = 0;	// OK
				break;
			}
		}
		if( loop != 0 ){
			// Timeout (10ms*100=1s)
			pr_err("buffer clearing time out\n");
			return ;
		}
		else{
			// Flash rom writing finish
			pr_err("buffer clearing complete\n");
		}


		//¢ß	The data of 64Kbyte is transmitted to RAM for the buffer of M-7MO. (The address of RAM for the buffer is 0x6800_0000 ¨C 0x6800_FFFF.)
		//pr_err(" send 0x%08X-0x%08X...", flash_addr,( flash_addr + write_unit -1 ));
		M7MO_WriteMemory((unsigned char *)read_addr, send_buf_size, M7MO_FW_M7MORAM_ADDRESS, write_unit);
		pr_err("OK, ");

		//¢à	The FlashROM writing command is issued
		pr_err("writing...");
		M7MO_WriteOneByteCRAM( 0x0F, 0x07, 0x01 );

		//¢àpolling the completion of writing.
		for( loop = 0 ; loop < 200 ; loop++ ){
			pr_err(".");
			M7MO_Custom_Delay( 10 );
			ret = M7MO_ReadOneByteCRAM( 0x0F, 0x07 );
			if( ret == 0x00 ){
				loop = 0;	// OK
				break;
			}
		}
		if( loop != 0 ){
			// Timeout (10ms*200=2s)
			pr_err("writing time out\n");
			return ;
		}
		else{
			// Flash rom writing finish
			pr_err("writing complete\n");
		}

		// Prepare for next iteration
		flash_addr += write_unit;
		read_addr  += write_unit;
		write_size -= write_unit;
	}
	return ;


}



/**
 *	@brief		Get the check sum
 *  @param[out]	sum				the checksum
 *	@return		0				OK
				-1				Fail
 *	@note		None
 */
static int M7MO_GetCheckSum(unsigned short * sum )
{
	int	loop, ret, acc;
	unsigned long	chk_addr, chk_size, set_size;
	unsigned short	ret_sum;

	chk_addr = WRITE_BUF_ADDR;
	chk_size = WRITE_BUF_SIZE;
	ret_sum  = 0;
	acc = 0x02;	// 16bit unit

	while( chk_size >0 ){
		if( chk_size >= M7MO_HOST_SUM_MAXSIZE ){
			set_size = M7MO_HOST_SUM_MAXSIZE;
		}
		else{
			set_size = chk_size;
		}
		// Set the start address
		M7MO_WriteOneWordCRAM( 0x0F, 0x00, chk_addr );
		// Set the size for checksum
		M7MO_WriteOneHalfwordCRAM( 0x0F, 0x04, set_size );
		// Start to get the checksum
		M7MO_WriteOneByteCRAM( 0x0F, 0x09, acc );
		// Wait for getting the checksum
		for( loop = 0 ; loop < 200 ; loop++ ){
			M7MO_Custom_Delay( 5 );
			ret = M7MO_ReadOneByteCRAM( 0x0F, 0x09 );
			if( ret == 0x00 ){
				ret_sum += M7MO_ReadOneHalfwordCRAM( 0x0F, 0x0A );
				break;
			}
		}
		if( loop >= 200 ){
			return	-1;
		}
		// Next iteration
		chk_addr += set_size;
		chk_size -= set_size;
	}
	*sum = ret_sum;
	return	0;
}
#endif



// TODO customize the following function
/**
 *	@brief		Open firmware bin file and fill into a buffer and returns its address and size.
 *	@param[out]	buf_addr		the address of buffer containing the loaded firmware
 *	@param[out]	buf_size		the size of the buffer
 *	@return		0				firmware loaded successfully
				-1				failed to load firmware 
 *	@note 		The READ_BUF_ADDR will become INVALID after calling the following function
 *				since it will be overridden by I2C WriteMemory command headers
 */
int M7MO_Custom_LoadFirmware(unsigned long * buf_addr, unsigned long * buf_size)
{	
	int ret = 0;

  
#if 1		
	
	struct file * file_fd = 0;
    char filepath[128];
    
	// clear memory buffer at READ_BUF_ADDR
	// TODO change to the available memory clear function
	char *buf=vmalloc(2.5*1024*1024);
	memset((char*)buf, 0xFF, READ_BUF_SIZE );


   
	memset(filepath, 0, sizeof(filepath));

	sprintf(filepath, "/system/etc/%s", "RS_M7MO.bin");

	pr_err("%s entry", __func__);
	

    file_fd = klib_fopen(filepath, O_RDWR, S_IALLUGO);
    if(file_fd ==0)
    {
    pr_err("isp klib_fopen error ");
	}
	

    ret = klib_fread((char *)buf + M7MO_WRITEMEMORY_START , READ_BUF_SIZE, file_fd);     
  
	
	if( ret < 0 ){	// File not found
		pr_err("Can't read the binary file.(%s)\n", filepath );
		return 0;
	}
	
	
#endif


	*buf_addr = (unsigned long )buf;
	*buf_size = READ_BUF_SIZE;


	return ret;
}


/**
 * Redirect fw write call to client custom codes
 * @return 0:OK, 1:NG
 */
int M7MO_FwWrite(void)
{
	int	ret;
	unsigned short	write_sum = 0;
	unsigned long buf_addr, buf_size;

    pr_err("%s entry", __func__);
    	
	// Load firmware into a buffer and get the buffer address and size
	ret = M7MO_Custom_LoadFirmware(&buf_addr, &buf_size);
    if (ret < 0 )
    {
	    pr_err("Firmware not found .\n");
        return ret;
    }
	


	// Write the sectors
	M7MO_FwWriteMain(buf_addr, buf_size);

	// Checksum
	// write_sum = 0: OK, otherwise: NG
	M7MO_GetCheckSum(&write_sum);
	
	pr_err("CheckSum is 0x%04X\n", write_sum);
	pr_err("Firmware write %s.\n", write_sum==0? "succeeded":"failed");
	
	ret = (write_sum==0? 0:-1);


     pr_err("write_sum =%d \n",write_sum);

	  

	return ret;
}



//M7MO_Custom\M7MO_Custom_Interrupt.c
//#define M7MO_CUSTOM_DEFAULT_POLL_TIME		20 		// 20 ms
//#define M7MO_CUSTOM_DEFAULT_POLL_COUNT		300		// 300 times

//M7MO_Function\M7MO_ChangeMode.c

/**
 *	@brief		Change M7MO system mode.
 *  @param[in]	mode		the target mode
 *	@return		0:OK, 1:NG
 */
int M7MO_ChangeMode(E_M7MO_Mode mode)
{
	int parameter;

	switch(mode){
	case E_M7MO_Mode_Parameter:
		parameter = M7MO_PARA_MODE_PAR;
		break;
	case E_M7MO_Mode_Monitor:
		parameter = M7MO_PARA_MODE_MON;
		break;		
	case E_M7MO_Mode_Capture:
		parameter = M7MO_PARA_MODE_CAP;
		break;		
	default:
		parameter = M7MO_PARA_MODE_ERR;	
		break;
	}

	if(parameter != M7MO_PARA_MODE_ERR)
	{
		int intFactor;

		//Enable all type of interrupt factor
		//Write: INT_EN_MODE = '0xFF' (i.e. write category:0x00, byte: 0x10 to 0xFF)
		M7MO_WriteOneByteCRAM(0x00, 0x10, 0xff);

		//Enable interrupt signal
		//Write: INT_ROOT_ENABLE = '0x01' (i.e. write category:0x00, byte: 0x12 to 0x01)
		M7MO_WriteOneByteCRAM(0x00, 0x12, 0x01);		

		//Go to the specified mode
		//Write: SYS_MODE = parameter (i.e. write category:0x00, byte: 0x0B to parameter)
		M7MO_WriteOneByteCRAM(0x00, 0x0B, parameter);		

		//Wait interrupt 
		if(mode == E_M7MO_Mode_Monitor)
		{
			intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_MON, M7MO_WAIT_AND);
			return (intFactor? M7MO_OK:M7MO_NG);
		}
		else if(mode == E_M7MO_Mode_Capture)
		{
			intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_CAPTURE, M7MO_WAIT_AND);			
			return (intFactor? M7MO_OK:M7MO_NG);
		}
		else
		{
			// E_M7MO_Mode_Parameter
			// TODO Confirm 
			return 0;
		}
	}	
	return 1;
}

//M7MO_Function\M7MO_SetClock.c


/**
 * Change the YUV clock frequency.
 * Call this function after changing to capture mode and before 
 * transfering capture preview image.
 * @param[in] option 	the target frequency of the YUV clock
 * @note Availability:
 * Flash Writer [--] Parameter [--] Monitor [--] Capture Preview [OK]
 */
void M7MO_SetYUVClock(E_M7MO_YUVClockOption option)
{
	unsigned char write_buffer[16];

	// disable interrupt
	// - set category 0 byte 0x12 to 0
	M7MO_WriteOneByteCRAM(0x00, 0x12, 0x01);

	// switch off the clock
	// - set the bit 7 of the byte at 0x5000003D to 1 
	write_buffer[M7MO_WRITEMEMORY_START] = 0x80;
	M7MO_WriteMemory(write_buffer, sizeof(write_buffer), 0x5000003D, 1);

	// change the clock frequency
	write_buffer[M7MO_WRITEMEMORY_START] = (unsigned char) option;
	M7MO_WriteMemory(write_buffer, sizeof(write_buffer), 0x5000000A, 1);

	// switch on the clock
	// - clear the bit 7 of the byte at 0x5000003D
	write_buffer[M7MO_WRITEMEMORY_START] = 0x00;
	M7MO_WriteMemory(write_buffer, sizeof(write_buffer), 0x5000003D, 1);

	// enable interrupt
	// - set category 0 byte 0x12 to 1
	M7MO_WriteOneByteCRAM(0x00, 0x12, 0x01);
}


//M7MO_Function\M7MO_Capture.c


/**
 * Set the maximum JPEG size for both thumbnail and jpeg
 * @param[in] size		the maximum size (in byte)
 * 						0x100000: 1MB
 * 						0x300000: 3MB
 * 						0x500000: 5MB
 */
void M7MO_Capture_SetMaxJPEGSize(unsigned long size)
{
	M7MO_WriteOneWordCRAM(0x0B, 0x0F, size);	
}

void M7MO_Capture_SetMinJPEGSize(unsigned long size)
{
	M7MO_WriteOneWordCRAM(0x0B, 0x13, size);
}

/**
 * Get the maximum JPEG size 
 */
unsigned long M7MO_Capture_GetMaxJPEGSize(void)
{
	return M7MO_ReadOneWordCRAM(0x0B, 0x0F);
}

/**
 * @brief		Return the thumbnail jpeg file size
 * @return thumbnail jpeg file size
 */
int M7MO_Capture_GetThumbnailSize(void)
{
	//Get Thumbnail image JPEG size
	//READ: THM_JPEG_SIZE (i.e. read category: 0x0C, byte: 0x11~0x14)
	return M7MO_ReadOneWordCRAM(0x0C, 0x11);	
}

/**
 * @brief		Return the main jpeg file size
 * @return main jpeg file size
 */
int M7MO_Capture_GetMainSize(void)
{
	//READ: IMAGE_SIZE (i.e. read category: 0x0C, byte: 0x0D~0x10)
	return M7MO_ReadOneWordCRAM(0x0C, 0x0D);	
}



//M7MO_Function\M7MO_SingleCap.c
/**
 * @brief		Return the main jpeg file size
 * @return main jpeg file size
 */
int M7MO_SingleCap_GetMainSize(void)
{
	return M7MO_ReadOneWordCRAM(0x0C, 0x0D);				//(category: 0x0C, byte: 0x0D~0x10)
}


/**
 *	@brief		Request M7MO to capture one image
 *  @param[in]	En_AHS, 0:Disable 1:Enable
 *	@return		0:OK, 1:NG
 *	@note		None
 *	@attention	None
 */
int M7MO_SingleCap_Start(unsigned char En_AHS)
{
	int intFactor;

	//Set Capture-mode
	if(En_AHS){
		M7MO_WriteOneByteCRAM(0x0C, 0x00, 0x02);		//Write: CAP_MODE = 0x02: Still capture anti-hand shake mode(Auto)
	}
	else{
		M7MO_WriteOneByteCRAM(0x0C, 0x00, 0x00);		//Write: CAP_MODE = 0x00: Normal (single capture)
	}

	//Enable all type of interrupt factor
	M7MO_WriteOneByteCRAM(0x00, 0x10, 0x88);		//Write: INT_EN_MODE = '0xFF' (i.e. write category:0x00, byte: 0x10 to 0xFF)

	//Go to capture mode
	M7MO_WriteOneByteCRAM(0x00, 0x0B, 0x03);		//Write: SYS_MODE = '0x03' (i.e. write category:0x00, byte: 0x0B to 0x03)

	//Wait shutter sound interrupt
	//Wait interrupt (INT_STATUS_SOUND) from M7MO
	intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_SOUND, M7MO_WAIT_AND);	

	if(!intFactor){
		return M7MO_NG;	
	}

	//Wait capture interrupt
	//Wait interrupt (INT_STATUS_CAPTURE) from M7MO
	intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_CAPTURE, M7MO_WAIT_AND);

	if(!intFactor){
		return M7MO_NG;
	}
	
	return M7MO_OK;
}

/**
 *	@brief		Request M7MO to output YUV preview
 */
void M7MO_SingleCap_GetPreview(void)
{
	//Select image number of frame for Preview image.
	M7MO_WriteOneByteCRAM(0x0C, 0x07, 0x01);		// When Single Capture, Specify "1".

	//Set YUV out for Preview
	M7MO_WriteOneByteCRAM(0x0B, 0x05, 0x00);		// Ex. YUV422

	//Set Preview Image size
	M7MO_WriteOneByteCRAM(0x0B, 0x06, 0x01);		// Ex. 320x240
	
	//Get Preview data
	M7MO_WriteOneByteCRAM(0x0C, 0x09, 0x02);		// Start to send Preview data from M7MO
}


/**
 *	@brief		Request M7MO to output main jpeg
 */
void M7MO_SingleCap_GetMain(void)
{
	//Select image number of frame for Main image
	M7MO_WriteOneByteCRAM(0x0C, 0x06, 0x01);		// When Single Capture, Specify "1"
#if 0
	//Set main image JPEG file man size
	M7MO_Capture_SetMaxJPEGSize(0x200000);			// Ex. Min Size = 2M Byte

	//Set main image JPEG file min size
	M7MO_Capture_SetMinJPEGSize(0x100000);			// Ex. Min Size = 1M Byte

	//Select main image format
	M7MO_WriteOneByteCRAM(0x0B, 0x00, 0x01);		// Ex. JPEG(with header:422)

	//Select main image size
	M7MO_WriteOneByteCRAM(0x0B, 0x01, 0x2C);		// Ex. 4128 x 3096 (13M)
#else
	//Select main image format
	M7MO_WriteOneByteCRAM(0x0B, 0x00, 0x00);		// YUV422
	
#if 0 //wt 5m yuv 2012 -0511
	//Select main image size
	M7MO_WriteOneByteCRAM(0x0B, 0x01, 0x20);		// 2592 x 1944 (5M)
#endif

	M7MO_WriteOneByteCRAM(0x0B, 0x01, 0x2a);		// 4000 x 3000(12M)
	

	pr_err("wt set image size 0x2a \n");
#endif

	//Get main image data
	M7MO_WriteOneByteCRAM(0x0C, 0x09, 0x01);		// Start to send main image data from M7MO
    pr_err("wt get image data");

}



/**
 * @brief		Return the thumbnail jpeg file size
 * @return thumbnail jpeg file size
 */
int M7MO_SingleCap_GetThumbnailSize(void)
{
	return M7MO_ReadOneWordCRAM(0x0C, 0x11);				//(category: 0x0C, byte: 0x11~0x14)
}



/**
 * @brief		Return the main jpeg file size
 * @return main jpeg file size
 */
int M7MO_SingleCap_GetThumbnail(void)
{
	return M7MO_ReadOneWordCRAM(0x0C, 0x0D);				//(category: 0x0C, byte: 0x0D~0x10)
}

/**
 * @brief		Single capture command sequence
 * @return		0:OK, 1:NG
 */
int M7MO_SingleCap(void)
{
	int intFactor = 1;
	int acceptFile=1;
	//unsigned long mainSize;
#ifdef M7MO_OUTPUT_THUMBNAIL // Output thumbnail
	unsigned long thumbSize;
#endif
	
	// No anti-handshaking normal capture
	if(!M7MO_SingleCap_Start(M7MO_FALSE)){
		pr_err("Start single capture OK\n");

#ifdef M7MO_OUTPUT_PREVIEW // Output preview
		//TODO Prepare to get preview image
		HostCustom_GetPreviewInit();

		//Call M7MO to send preview image
		M7MO_SingleCap_GetPreview();

		//TODO Get preview image
		//ASSERT(HostCustom_GetPreviewCore());

		//Wait interrupt for Data Transfer Complete
		intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_CAPTURE, M7MO_WAIT_AND);
#endif

		if(intFactor){
#ifdef M7MO_OUTPUT_PREVIEW // Output preview
			//TODO display the preview image out
			HostCustom_ShowPreview();
#endif
			//TODO check if the image should be saved, or discarded
			//Host has to check if it is required to output thumbnail and Jpeg
			//(e.g. User may not want to save the image after seeing the preview)
			//acceptFile = HostCustom_IsSaveImage();

			if(acceptFile)
			{
#ifdef M7MO_OUTPUT_THUMBNAIL // Output thumbnail
				//TODO Prepare to get thumbnail image
				HostCustom_GetThumbInit();

				//Call M7MO to send thumbnail image
				M7MO_SingleCap_GetThumbnail();

				//TODO Get thumbnail image
				ASSERT(HostCustom_GetThumbCore());

				//Wait interrupt for Data Transfer Complete
				intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_CAPTURE, M7MO_WAIT_AND);
#endif

				if(intFactor){
#ifdef M7MO_OUTPUT_THUMBNAIL // Output thumbnail
					//Get Thumbnail image JPEG size
					thumbSize = M7MO_SingleCap_GetThumbnailSize();

					//TODO Host could save the files later to speed up the capture process
					HostCustom_SaveThumb(thumbSize);
#endif
					//TODO Prepare to get main image
					HostCustom_GetMainInit();

					pr_err("Start Data tranfer\n");

					//Call M7MO to send main image
					M7MO_SingleCap_GetMain();

					//TODO Get the main image
					HostCustom_GetMainCore();

					//Wait interrupt for Data Transfer Complete
					intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_CAPTURE, M7MO_WAIT_AND);
					pr_err("Data tranfer Complete\n");
#if 0
					if(intFactor){
						//Get JPEG size
						mainSize = M7MO_SingleCap_GetMainSize();

						//TODO Read Exif information
						//Read (Category: 0x07, Byte:0x00~0x2F)

						//TODO Host saves the received thumbnail and jpeg
						HostCustom_SaveMain(mainSize);
					}
#endif					
				}
			}
		}
	}else{
		pr_err("Start single capture failed\n");
	}
	// Got back to monitor mode after capture
	//M7MO_ChangeMode(E_M7MO_Mode_Monitor);
	pr_err("Start single capture completer \n");

	return 0;
}


/**
 *	@brief		Request M7MO to Start CAF
 *  @param[in]	None
 *  @return
 *	@note		None
 *	@attention	Could only be executed during monitor mode
 */

void M7MO_CAF_Start(void )
{
	pr_err("%s	   ",__func__);

	//Set AF window
	M7MO_WriteOneByteCRAM(0x0A, 0x2A, 0x01);

	//Set AF scan mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2B, 0x04);
	
	//Set AF range mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2C, 0x00);

	//Start auto focus
	M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x01);

}


/**
 *	@brief		Request M7MO to Stop CAF
 *  @param[in]	None
 *  @return
 *	@note		None
 *	@attention	Could only be executed during monitor mode
 */
int M7MO_CAF_Stop(void)
{
	unsigned char intFactor;
	pr_err("%s	   ",__func__);

	//Stop auto focus
	M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x00);

	//Wait interrupt
	intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_AF, M7MO_WAIT_AND);		

	if(!intFactor){
		return 1;
	}
	else
		return 0;
}



int m7mo_set_focusemode_macro(void)
{
	int rc=0;
	pr_err("%s	   ",__func__);

	#if 0
	//Set AF window
	M7MO_WriteOneByteCRAM(0x0A, 0x2A, 0x01);

	//Set AF scan mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2B, 0x03);
	#endif
	
	//Set AF range mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2C, 0x01);


	//Start auto focus
	//M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x01);

	
	return rc;

}

E_M7MO_AF_WINDOW HOST_Get_AF_Window_Mode(void)
{
	return E_M7MO_AF_WINDOW_CENTER_LARGE;
}


E_M7MO_AF_SCAN_MODE HOST_Get_AF_Scan_Mode(void)
{
	return E_M7MO_AF_SCAN_MODE_FAST;
}


E_M7MO_AF_RANGE_MODE HOST_Get_AF_Range_Mode(void)
{
	return E_M7MO_AF_RANGE_MODE_NORMAL;
}

unsigned short HOST_Get_Touch_Window_X(void)
{
	return touchafaex;
}

unsigned short HOST_Get_Touch_Window_Y(void)
{
	return touchafaey;
}

unsigned short HOST_Get_Touch_Window_W(void)
{
	return touchdafaedx;
}

unsigned short HOST_Get_Touch_Window_H(void)
{
	return touchdafaedx;
}

unsigned short HOST_Get_Monitor_Size_W(void)
{
    int width=640;
	
	pr_err("%s     width  =%d   ",__func__,width);

	return  width;   
}

unsigned short HOST_Get_Monitor_Size_H(void)
{
    int height=480;
	
	pr_err("%s	height =%d 	",__func__,height);

	return  height;
}
/**
 *	@brief		Request M7MO to execute auto focus
 *  @param[in]	None
 *  @return 	0:OK, 1:AF Release failed, 2: AF Focus failed
 *	@note		None
 *	@attention	Could only be executed during monitor mode
 */
 #if 0
int M7MO_AF_Start(int M7MO_AF_MODE)
{
	unsigned char intFactor;
	unsigned char afResult;

	pr_err("af new_samecode %s      ",__func__);
	
	//Enable all type of interrupt factor
	M7MO_WriteOneByteCRAM(0x00, 0x10, 0xff);		//Write: INT_EN_MODE = '0xFF' (i.e. write category:0x00, byte: 0x10 to 0xFF)

	//Enable interrupt signal
	M7MO_WriteOneByteCRAM(0x00, 0x12, 0x01);		//Write: INT_ROOT_ENABLE = '0x01' (i.e. write category:0x00, byte: 0x12 to 0x01)

	//Release auto focus
	M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x00);		//Write: AF_START = '0x00' (i.e. write category:0x0A, byte: 0x02 to 0x00)


	//Wait interrupt
	intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_AF, M7MO_WAIT_OR);

	if(!intFactor){
		// AF Release failed
		pr_err("AF Release failed");
		return 1;
	}
	
	//Set AF window mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2A, HOST_Get_AF_Window_Mode());

	//Set AF scan mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2B, HOST_Get_AF_Scan_Mode());
	
	//Set AF range mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2C, HOST_Get_AF_Range_Mode());



	//Start auto focus
	M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x01);		//Write: AF_START = '0x01' (i.e. write category:0x0A, byte: 0x02 to 0x01)

	//Wait interrupt
	intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_AF, M7MO_WAIT_AND);		

	//Read AF result
	//READ: AF_RESULT (i.e. read category: 0x0A, byte: 0x03)
	afResult = M7MO_ReadOneByteCRAM(0x0A, 0x03);	

	if(afResult == 1)
	{
	pr_err("%s ,AF ok ",__func__);
	 return 0;
	}
	else 
	{
	pr_err("%s ,AF  failed",__func__);
		return 2;
	}
}

#else
int M7MO_AF_Start(E_M7MO_AF_MODE af_mode)
{
	unsigned char intFactor;
	unsigned char afResult;
	pr_err("af older samecode %s:   X    %d\n", __func__, __LINE__);
	//Enable all type of interrupt factor
	M7MO_WriteOneByteCRAM(0x00, 0x10, 0xff);		//Write: INT_EN_MODE = '0xFF' (i.e. write category:0x00, byte: 0x10 to 0xFF)

	//Enable interrupt signal
	M7MO_WriteOneByteCRAM(0x00, 0x12, 0x01);		//Write: INT_ROOT_ENABLE = '0x01' (i.e. write category:0x00, byte: 0x12 to 0x01)

	//Release auto focus
	M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x00);		//Write: AF_START = '0x00' (i.e. write category:0x0A, byte: 0x02 to 0x00)

	//Wait interrupt
	intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_AF, M7MO_WAIT_AND);		

	if(!intFactor){
		// AF Release failed
		pr_err("%s:   X    %d\n", __func__, __LINE__);
		return 1;
	}
	pr_err("%s:   X    %d\n", __func__, __LINE__);
	//Set AF mode
	M7MO_WriteOneByteCRAM(0x0A, 0x2B, af_mode);
	
	//Enable interrupt signal
	M7MO_WriteOneByteCRAM(0x00, 0x12, 0x01);		//Write: INT_ROOT_ENABLE = '0x01' (i.e. write category:0x00, byte: 0x12 to 0x01)

	//Start auto focus
	M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x01);		//Write: AF_START = '0x01' (i.e. write category:0x0A, byte: 0x02 to 0x01)

	//Wait interrupt
	intFactor = M7MO_WaitInterrupt(M7MO_HOSTINT_AF, M7MO_WAIT_AND);		

	//Read AF result
	//READ: AF_RESULT (i.e. read category: 0x0A, byte: 0x03)
	afResult = M7MO_ReadOneByteCRAM(0x0A, 0x03);	

	if(afResult == 1)
	{	pr_err("%s:   X  af ok  %d\n", __func__, __LINE__);
		return 0;
	}
	else
	{ 
		pr_err("%s:   X  af error  %d\n", __func__, __LINE__);
		return 2;
	}
}
#endif
/**
 *	@brief		Set touch window for AF/AE
 */
int M7MO_AFAE_Set_Touch_Window(void )
{

	unsigned short x, y, w, h;
	int result_5c,result_5e,result_60;

    pr_err("%s",__func__);
	
	// Set AE window weight
	M7MO_WriteOneByteCRAM(0x02, 0x65, 3);

	// Set AF window mode to use touch window
	M7MO_WriteOneByteCRAM(0x0A, 0x2A, 4);

	// it is necessary to disable AF AE window before window update
	M7MO_WriteOneByteCRAM(0x02, 0x64, 0);

	// get current user touch window and convert it as ratio to monitor size
	x = HOST_Get_Touch_Window_X()*256/HOST_Get_Monitor_Size_W();
	y = HOST_Get_Touch_Window_Y()*256/HOST_Get_Monitor_Size_H();
	w = HOST_Get_Touch_Window_W()*256/HOST_Get_Monitor_Size_W();
	h = HOST_Get_Touch_Window_H()*256/HOST_Get_Monitor_Size_H();

   pr_err(" x=%x ,y=%x,w=%x,h=%x",x,y,w,h);
	// update category touch area setting
	M7MO_WriteOneHalfwordCRAM(0x02, 0x5C, x);
	M7MO_WriteOneHalfwordCRAM(0x02, 0x5E, y);
	M7MO_WriteOneHalfwordCRAM(0x02, 0x60, w);
	M7MO_WriteOneHalfwordCRAM(0x02, 0x62, h);

#if 1 
	 //debug for toucha af 
	  result_5c=M7MO_ReadOneHalfwordCRAM(0x02,0x5C);
      result_5e=M7MO_ReadOneHalfwordCRAM(0x02,0x5E);
	  result_60=M7MO_ReadOneHalfwordCRAM(0x02,0x60);
	 pr_err("result_5c =%x  , result_5e =%x ,result_60 =%x",result_5c,result_5e,result_60);
#endif

	
	// update and apply new window setting
	M7MO_WriteOneByteCRAM(0x02, 0x64, 1);

	if (M7MO_ReadOneByteCRAM(0x02, 0x64) == 0)
	{
		// touch area is invalid, return fail
		pr_err("%s touch area is invalid",__func__);
		touchafae_state=0;
		return 0;
	}
	pr_err("%s touch area is  valid",__func__);

	// At this stage, the new AE window has already been active
	// issue the following command to use the new window to execute AF operation
	//M7MO_WriteOneByteCRAM(0x0A, 0x02, 0x01);		//Write: AF_START = '0x01' (i.e. write category:0x0A, byte: 0x02 to 0x01)
    touchafae_state=1;
	return 1;
}




#if 1


static ssize_t isp_fwupdate_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
    int isp_ver_h=0;
	int isp_ver_l=0;
	int i, ret;
	
	pr_err("%s: %d\n", __func__, __LINE__);
		
	msm_isp_cam_fwup_init(isp_i2c_client,isp_id);
	pr_err(" :%s: %d\n", __func__, __LINE__);
    msleep(100);

    //isp init
    
	//Write: CAM_START = '0x01' (i.e. write category:0x0F, byte: 0x12 to 0x01)
	M7MO_WriteOneByteCRAM(0x0F, 0x12, 0x01); 
	M7MO_Custom_Delay(50);
	//Read system mode until a normal value instead of 0xFF is returned 
	for( i=0 ; i<2500; i++ ){
		M7MO_Custom_Delay(10);
		ret = M7MO_ReadOneByteCRAM(0x00, 0x0B);
		if(( ret & 0xF0 )!= 0xF0 ){
           pr_err(" isp M7MO_System_Init M7MO_OK \n");

		  break;
		}
	}
	

	//version 
	isp_ver_h=M7MO_ReadOneByteCRAM(0x00, 0x02);
	isp_ver_l=M7MO_ReadOneByteCRAM(0x00, 0x03);
	
	snprintf(buf,25, "isp ver: 0x%02x%02x\n", isp_ver_h,isp_ver_l);
	pr_err("%s len=%d",buf,strlen(buf));
	   

    return strlen(buf);
}

//upgrade from app.bin
static ssize_t isp_fwupdate_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
   static int i=1;
   if(i==1)
   {
    pr_err("echo buf=%s \n",buf);
	msm_isp_cam_fwup_init(isp_i2c_client,isp_id);
	
	pr_err(" :%s: %d\n", __func__, __LINE__);
    msleep(100);


  	 isp_fw_wr_flags =(0== M7MO_FwWrite());
	 pr_err("isp_fw_wr_flags =%d\n",isp_fw_wr_flags);

	  M7MO_System_Init();
	  usleep_range(600, 600);

	   M7MO_VER();
	   
     i=0;
   	}
	return 0;
}

static DEVICE_ATTR(ispfwupdate, 0664, isp_fwupdate_show, isp_fwupdate_store);

extern struct kobject *firmware_kobj;

/*
return value
0, success
-1, fail
*/
int isp_fwupdate(struct i2c_client *client, char *pfwfilename ) 
{
	pr_err("%s: %d\n", __func__, __LINE__);

	if ( !client || !pfwfilename )
		return -1;

	// return (syna_fw_update( client, pfwfilename));
	return 0;
}


int isp_fwupdate_init(struct i2c_client *client,
	const struct i2c_device_id *id)
{

	int ret;
	struct kobject * fts_fw_kobj=NULL;

	isp_i2c_client=client;
    isp_id=id;
	




	pr_err("%s: %d\n", __func__, __LINE__);
#if 0
	fts_fw_kobj = kobject_get(firmware_kobj);
	if (fts_fw_kobj == NULL) {
		fts_fw_kobj = kobject_create_and_add("ispfw", NULL);
		if (fts_fw_kobj == NULL) {
			pr_err("%s: subsystem_register failed\n", __func__);
			ret = -ENOMEM;
			return ret;
		}
	}
#endif
 	fts_fw_kobj = kobject_create_and_add("ispfw", NULL);
		if (fts_fw_kobj == NULL) {
			pr_err("%s: subsystem_register failed\n", __func__);
			ret = -ENOMEM;
			return ret;
		}
	
	ret=sysfs_create_file(fts_fw_kobj, &dev_attr_ispfwupdate.attr);

	if (ret) {
		pr_err("%s: sysfs_create_file failed\n", __func__);
		return ret;
	}
	// syna_i2c_client = client;
	//printk("%s, xiayc: client=%p syna_i2c_client=%p\n",__func__,client,syna_i2c_client);

	pr_info("%s:isp  sysfs_create_file  succeed!\n", __func__);
	





return 0;	
}


int isp_fwupdate_deinit(struct i2c_client *client)
{
	struct kobject * fts_fw_kobj=NULL;

	pr_err("%s: %d\n", __func__, __LINE__);

	fts_fw_kobj = kobject_get(firmware_kobj);
	if ( !firmware_kobj ){
		printk("%s: error get kobject\n", __func__);
		return -1;
	}
	
	sysfs_remove_file(firmware_kobj, &dev_attr_ispfwupdate.attr);



	return 0;
}



#endif



static int32_t isp_cam_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	

	
	return 0;
}


static int isp_cam_powerdown(struct msm_sensor_ctrl_t *s_ctrl)
{
			int rc = 0;
	
			if (s_ctrl==NULL)
			{
				pr_err("%s =null",__func__);
				return 0;
			}


			rc = gpio_request(89,"ispcam");
					if (!rc) {
						pr_err("%s: gpio_request 89 OK\n", __func__);
						gpio_direction_output(89,0);
						usleep_range(1000, 2000);
						mdelay(1);
					} else {
						gpio_free(89);
						mdelay(1);
						rc = gpio_request(89,"ispcam");
						if (!rc){
						gpio_direction_output(89,0);
						usleep_range(1000, 2000);
						mdelay(1);
						}else{
						pr_err("%s: gpio_request 89 failed\n", __func__);
						}
					}

			rc = msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 0);
			if (rc < 0) {
				pr_err("%s: enable regulator failed\n", __func__);
				
			}
#if 0
			//back cam mipi DOVDD/DVDD
			rc = msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
									s_ctrl->sensordata->sensor_platform_info->cam_vreg,
									s_ctrl->sensordata->sensor_platform_info->num_vreg,
									s_ctrl->reg_ptr, 0);
			if (rc < 0) {
				pr_err("%s: regulator on failed\n", __func__);
				
			}
			
#endif
			rc = gpio_request(46,"ispcam");
			if (!rc) {
				pr_err("%s: gpio_request 46 OK\n", __func__);
				gpio_direction_output(46,1);
				usleep_range(1000, 2000);
				mdelay(1);
			} else {
				gpio_free(46);
				mdelay(1);
				rc = gpio_request(46,"ispcam");
				if (!rc){
				gpio_direction_output(46,1);
				usleep_range(1000, 2000);
				mdelay(1);
				}else{
				pr_err("%s: gpio_request 46 failed\n", __func__);
				}
			}

			rc = gpio_request(97,"ispcam");
			if (!rc) {
				pr_err("%s: gpio_request 97 OK\n", __func__);
				gpio_direction_output(97,0);
				usleep_range(1000, 2000);
				mdelay(1);
			} else {
				gpio_free(97);
				mdelay(1);
				rc = gpio_request(97,"ispcam");
				if (!rc){
				gpio_direction_output(97,0);
				usleep_range(1000, 2000);
				mdelay(1);
				}else{
				pr_err("%s: gpio_request 97 failed\n", __func__);
				}
			}

		
			rc = gpio_request(107,"ispcam");
			if (!rc) {
				pr_err("%s: gpio_request 107 OK\n", __func__);
				gpio_direction_output(107,0);
				usleep_range(1000, 2000);
				mdelay(1);
			} else {
				gpio_free(107);
				mdelay(1);
				rc = gpio_request(107,"ispcam");
				if (!rc){
				gpio_direction_output(107,0);
				usleep_range(1000, 2000);
				mdelay(1);
				}else{
				pr_err("%s: gpio_request 107 failed\n", __func__);
				}
					}
					
					
		
		
			
				
		
				
		
			pr_err("%s: %d\n", __func__, __LINE__);
			
		   
		
			
		
		
				
						
		
	
			mdelay(20);
		
		
	   return 0;
	
	
}


static int isp_cam_powerup(struct msm_sensor_ctrl_t *s_ctrl)
{
	    int rc = 0;

		if (s_ctrl==NULL)
		{
	        pr_err("%s =null",__func__);
			return 0;
		}
		
		rc = gpio_request(107,"ispcam");
		if (!rc) {
			pr_err("%s: gpio_request 107 OK\n", __func__);
			gpio_direction_output(107,0);
			usleep_range(1000, 2000);
			mdelay(1);
		} else {
			gpio_free(107);
			mdelay(1);
			rc = gpio_request(107,"ispcam");
			if (!rc){
			gpio_direction_output(107,0);
			usleep_range(1000, 2000);
			mdelay(1);
			}else{
			pr_err("%s: gpio_request 107 failed\n", __func__);
			}
				}
				
				
	
	
		rc = gpio_request(97,"ispcam");
			if (!rc) {
				pr_err("%s: gpio_request 97 OK\n", __func__);
				gpio_direction_output(97,1);
				usleep_range(1000, 2000);
				mdelay(1);
			} else {
				gpio_free(97);
				mdelay(1);
				rc = gpio_request(97,"ispcam");
				if (!rc){
				gpio_direction_output(97,1);
				usleep_range(1000, 2000);
				mdelay(1);
				}else{
				pr_err("%s: gpio_request 97 failed\n", __func__);
				}
			}
	
			rc = gpio_request(46,"ispcam");
			if (!rc) {
				pr_err("%s: gpio_request 46 OK\n", __func__);
				gpio_direction_output(46,0);
				usleep_range(1000, 2000);
				mdelay(1);
			} else {
				gpio_free(46);
				mdelay(1);
				rc = gpio_request(46,"ispcam");
				if (!rc){
				gpio_direction_output(46,0);
				usleep_range(1000, 2000);
				mdelay(1);
				}else{
				pr_err("%s: gpio_request 46 failed\n", __func__);
				}
			}
	
			
	
		pr_err("%s: %d\n", __func__, __LINE__);
		
	   //back cam mipi DOVDD/DVDD
		rc = msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
								s_ctrl->sensordata->sensor_platform_info->cam_vreg,
								s_ctrl->sensordata->sensor_platform_info->num_vreg,
								s_ctrl->reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: regulator on failed\n", __func__);
			
		}
	
		rc = msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
				s_ctrl->sensordata->sensor_platform_info->cam_vreg,
				s_ctrl->sensordata->sensor_platform_info->num_vreg,
				s_ctrl->reg_ptr, 1);
		if (rc < 0) {
			pr_err("%s: enable regulator failed\n", __func__);
			
		}
	
	
			rc = gpio_request(89,"ispcam");
				if (!rc) {
					pr_err("%s: gpio_request 89 OK\n", __func__);
					gpio_direction_output(89,1);
					usleep_range(1000, 2000);
					mdelay(1);
				} else {
					gpio_free(89);
					mdelay(1);
					rc = gpio_request(89,"ispcam");
					if (!rc){
					gpio_direction_output(89,1);
					usleep_range(1000, 2000);
					mdelay(1);
					}else{
					pr_err("%s: gpio_request 89 failed\n", __func__);
					}
				}
					
	

		mdelay(20);
	
	
   return 0;


}

 
int32_t msm_isp_cam_sensor_i2c_probe(struct i2c_client *client,
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

#ifdef ISP_CAM_FW_WRITER_MODE

	ispcam_s_ctrl = s_ctrl;
	isp_fwupdate_init(client,id);
	
#endif


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

	pr_err("%s: %d\n", __func__, __LINE__);
#if 1
	s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
				* s_ctrl->sensordata->sensor_platform_info->num_vreg, GFP_KERNEL);
		if (!s_ctrl->reg_ptr) {
			pr_err("%s: could not allocate mem for regulators\n",
				__func__);
			return -ENOMEM;
		}
		else
		{
			pr_err("%s: could allocate mem for regulators\n",
							__func__);

		}
#endif

  
   
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



	disable_isp_spi();

	goto power_down;



	probe_fail:
		pr_err("%s_i2c_probe failed\n", client->name);
		
	
	power_down:
		if (rc > 0)
			rc = 0;
		s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	

	pr_err("%s: %d\n", __func__, __LINE__);
	
	return rc;


}
static const struct i2c_device_id isp_cam_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&isp_cam_s_ctrl},
	{ }
};

static struct i2c_driver isp_cam_i2c_driver = {
	.id_table = isp_cam_i2c_id,
	.probe  = msm_isp_cam_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client isp_cam_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};
static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};




int32_t msm_isp_cam_fwup_init(struct i2c_client *client,
	const struct i2c_device_id *id)
{

	int rc = 0;

	struct msm_sensor_ctrl_t *s_ctrl;
	struct msm_camera_sensor_info *data ;

  	
	pr_err("%s_msm_isp_cam_fwup_init called\n", client->name);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality failed\n");
		rc = -EFAULT;
		return rc;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);

	
	data= s_ctrl->sensordata;

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

	pr_err("%s: %d\n", __func__, __LINE__);

   //isp power up
    rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
	return rc;

}


int32_t msm_isp_cam_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{

	int32_t rc = 0;
		struct msm_camera_sensor_info *data = s_ctrl->sensordata;
		pr_err("%s: %d\n", __func__, __LINE__);
		
		
		if(isp_boot_flags == 0 ){
            ispcam_vreg_enable();
			isp_cam_powerup(s_ctrl);
		}

		
		if (s_ctrl->clk_rate != 0)
			cam_clk_info->clk_rate = s_ctrl->clk_rate;
		
		pr_err("isp mclk %s: %d %ld \n", __func__, __LINE__,s_ctrl->clk_rate);
		
		rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
			cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
		if (rc < 0) {
			pr_err("%s: clk enable failed\n", __func__);
			goto enable_clk_failed;


		}
		mdelay(1);

		if(isp_boot_flags == 0 ){
		pr_err("isp mclk %s: clk enable ok \n", __func__);

		rc = gpio_request(107,"ispcam");
			if (!rc) {
				pr_err("%s: gpio_request 107  1 OK\n", __func__);
				gpio_direction_output(107,1);
				usleep_range(1000, 2000);
				mdelay(1);
			} else {
				gpio_free(107);
				mdelay(1);
				rc = gpio_request(107,"ispcam");
				if (!rc){
				gpio_direction_output(107,1);
				usleep_range(1000, 2000);
				mdelay(1);
				}else{
				pr_err("%s: gpio_request 107 1 failed\n", __func__);
				}
			}
			isp_boot_flags=1;
		pr_err("isp reset high  %s: reset high enable ok \n", __func__);
	}
		
		rc = msm_camera_request_gpio_table(data, 1);
		if (rc < 0) {
			pr_err("%s: request gpio failed\n", __func__);
			goto request_gpio_failed;
		}
		rc = msm_camera_config_gpio_table(data, 1);
		if (rc < 0) {
			pr_err("%s: config gpio failed\n", __func__);
			goto config_gpio_failed;
		}
	
		usleep_range(1000, 2000);
		if (data->sensor_platform_info->ext_power_ctrl != NULL)
			data->sensor_platform_info->ext_power_ctrl(1);
	
		return rc;
	
	config_gpio_failed:
	request_gpio_failed:
	enable_clk_failed:
			msm_camera_config_gpio_table(data, 0);
	
		return rc;

}


int32_t msm_isp_cam_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	pr_err("%s:  %d\n", __func__, __LINE__);
    if(isp_boot_flags==1)
	{
		isp_cam_powerdown(s_ctrl);
		ispcam_vreg_disable();
		
		msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);

		pr_err("%s:  %d\n", __func__, __LINE__);
		
		isp_boot_flags=0;
		isp_ini_flag=0;
	}
	
	return 0;
}

int32_t msm_isp_cam_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	int test_data;

	
	pr_err("%s: update_type= %d , %d\n", __func__,update_type, __LINE__);
	v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
		NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
		PIX_0, ISPIF_OFF_IMMEDIATELY));

	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		s_ctrl->curr_csi_params = NULL;
		rc=msm_sensor_enable_debugfs(s_ctrl);
		if (rc < 0) {
		pr_err("%s: msm_sensor_enable_debugfs  failed\n", __func__);
		}

            pr_err("MSM_SENSOR_REG_INIT %s: %d\n", __func__, __LINE__);
            if(0 == isp_ini_flag){
			M7MO_System_Init();
			//M7MO_WriteOneByteCRAM(0x01, 0x01,0x17); 640*480
			M7MO_WriteOneByteCRAM(0x01, 0x01,0x25);
			isp_ini_flag=1;
            }
			
		    //M7MO_WriteOneByteCRAM(0x0D, 0x1B,0x05); //corlor bar 
		
	} 
	else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		
		if((&s_ctrl->sensor_v4l2_subdev)->v4l2_dev->notify)
			pr_err("%s:notify is not null \n", __func__);
		
       if(s_ctrl->curr_csi_params != s_ctrl->csi_params[res]){
		pr_err("%s:mipi begin  res=%d\n", __func__,res);
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
		pr_err("%s:mipi end    res=%d\n", __func__,res);
       	}
	   
        //isp preview
		if(res==1){

		isp_cam_state=1;
		touchafae_state=0;
		 

		   #if 0 //tesmp for  color bar snap 2012 0510
			M7MO_WriteOneByteCRAM(0x0D, 0x1B,0x05); //corlor bar
			#endif
			
			M7MO_ChangeMode(E_M7MO_Mode_Monitor);
            /*  flash_torch test function */
			pr_err("isp flash \n");
			//M7MO_WriteOneByteCRAM(0x03, 0x34, 0x09);
			
			
			test_data= M7MO_ReadOneByteCRAM(0x00, 0x0c);
			pr_err("%s: M7MO_ReadOneWordCRAM ---1:: test_data= 0x%x\n", __func__, test_data); 
			
			test_data= M7MO_ReadOneByteCRAM(0x01, 0x00);
			pr_err("%s: M7MO_ReadOneWordCRAM ---2:: test_data= 0x%x\n", __func__, test_data);
		}


       	//snapshot 
		if(res==0){
			isp_cam_state=0;
			if(!M7MO_SingleCap_Start(M7MO_FALSE)){
				pr_err("Start single capture OK\n");
			}
		}	

	//jeff
	
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
			output_settings[res].op_pixel_clk);
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_ISPIF_STREAM, (void *)ISPIF_STREAM(
			PIX_0, ISPIF_ON_FRAME_BOUNDARY));

       	//snapshot 
		if(res==0){
		
	    pr_err("%s:isp snapshot  res=%d\n", __func__,res);	

		//Call M7MO to send main image
		M7MO_SingleCap_GetMain();
		
		//Wait interrupt for Data Transfer Complete
		//M7MO_WaitInterrupt(M7MO_HOSTINT_CAPTURE, M7MO_WAIT_AND);
		//pr_err("Data tranfer Complete\n");

		
		pr_err("%s:  isp snapshot   X    %d\n", __func__, __LINE__);
	    
       }
		

	}
	return rc;
}



int ispcam_set_flash_led_torch (struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	static int flash_torch=0;
	
	pr_err("isp   %s   value:  %d  flash_torch %d \n", __func__,value,flash_torch);

	switch (value)
	{
	case 0:
		if(flash_torch==1)
		{
		 M7MO_WriteOneByteCRAM( 0x03, 0x34, 0x00 );
		 pr_err("%s   torch   off   \n",__func__);
		}
		else
		{
		 M7MO_WriteOneByteCRAM( 0x03, 0x35, 0x00 );
		 pr_err("%s   flash   off   \n",__func__);
		}
		
    	
     break;

	case 1:
		 M7MO_WriteOneByteCRAM( 0x03, 0x35, 0x02 );
		pr_err("flash  auto");
		flash_torch=0;
 		
	   break;
	   
	 case 2:
		 M7MO_WriteOneByteCRAM( 0x03, 0x35, 0x01 );
		pr_err("flash  on");
		flash_torch=0;
		
     break;
		 
	 case 3:
		 M7MO_WriteOneByteCRAM( 0x03, 0x34, 0x06 );//125ma
		pr_err("flash  torch");	
		flash_torch=1;
	
	   break;

	 
	   
	   
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}



int ispcam_set_sharpness(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);

	switch (value)
	{
	case 0:
		 pr_err("%s   sharpness   off   \n",__func__);
    	 M7MO_WriteOneByteCRAM(0x02, 0x16, 0x00);
     break;

	case 5:
		 pr_err("%s sharpness  1 \n",__func__);
 		M7MO_WriteOneByteCRAM(0x02, 0x16, 0x01);
		M7MO_WriteOneByteCRAM(0x02, 0x18, 0x01);
	   break;
	   
	 case 10:
		 pr_err("%s sharpness  2 \n",__func__);
		M7MO_WriteOneByteCRAM(0x02, 0x16, 0x01);
		M7MO_WriteOneByteCRAM(0x02, 0x18, 0x03);
     break;
		 
	 case 15:
		 pr_err("%s sharpness  3 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x16, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x18, 0x05);

	
	   break;

	 case 20:
		 pr_err("%s sharpness  4 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x16, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x18, 0x07);

	
	   break;


	  case 25:
		 pr_err("%s sharpness  5 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x16, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x18, 0x09);

	
	   break;

	   case 30:
		 pr_err("%s sharpness  6 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x16, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x18, 0x0C);

	
	   break;
	   
	   
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}





int ispcam_set_saturation(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);

	switch (value)
	{
	case 0:
		 pr_err("%s   saturation   off   \n",__func__);
		M7MO_WriteOneByteCRAM(0x02, 0x10, 0x00);

     break;

	case 1:
		 pr_err("%s saturation  1 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x10, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x0F, 0x01);

	   break;
	   
	 case 2:
		 pr_err("%s saturation  2 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x10, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x0F, 0x02);

     break;
		 
	 case 3:
		 pr_err("%s saturation  3 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x10, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x0F, 0x03);

	
	   break;

	 case 4:
		 pr_err("%s saturation  4 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x10, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x0F, 0x04);

	
	   break;


	  case 5:
		 pr_err("%s saturation  5 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x02, 0x10, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x0F, 0x05);

	
	   break;

	   case 6:
		 pr_err("%s saturation  6 \n",__func__);

		 M7MO_WriteOneByteCRAM(0x02, 0x10, 0x01);
		 M7MO_WriteOneByteCRAM(0x02, 0x0F, 0x05);
	   break;
	   
	   
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}





int ispcam_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);

	switch (value)
	{
	case 0:
		 pr_err("%s   antibanding   off   \n",__func__);
    	M7MO_WriteOneByteCRAM(0x03, 0x06, 0x04);
     break;

	case 1:
		 pr_err("%s antibanding  60hz \n",__func__);
 		M7MO_WriteOneByteCRAM(0x03, 0x06, 0x02);
	   break;
	   
	 case 2:
		 pr_err("%s antibanding  50hz \n",__func__);
		 M7MO_WriteOneByteCRAM(0x03, 0x06, 0x01);
     break;
		 
	 case 3:
		 pr_err("%s antibanding  auto \n",__func__);
		 M7MO_WriteOneByteCRAM(0x03, 0x06, 0x00);
	
	   break;
	   
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}
int ispcam_set_redeye_reduce(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);

	//M7MO_VER();

	switch (value)
	{
	case 0:
		 pr_err("%s   red-eye-reduce close \n",__func__);
   
			

		break;

	case 1:
		 pr_err("%s   red-eye-reduce opens 1 \n",__func__);
        
			
	
	   break;
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}


int ispcam_set_exposure(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
		//int16_t  tmp_reg = 0;
		pr_err("isp   %s   value:  %d\n", __func__,value);

	switch (value)
	{
	case 0:
		 pr_err("%s   frame-average 0 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x03, 0x01, 0x01);
       
    	break;

	case 1:
		 pr_err("%s   center-weighted 1 \n",__func__);
		 M7MO_WriteOneByteCRAM(0x03, 0x01, 0x02);

		break;
		
	case 2:
		 pr_err("%s   spot-metering 2 \n",__func__);	
		 M7MO_WriteOneByteCRAM(0x03, 0x01, 0x05);

		break;
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}
int ispcam_set_touch_af_ae( struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, uint32_t value)
{
	int rc = 0;
	int x=value>>21;
	int y=(value&0x1ffc00)>>11;   //(value&~(0x7FF<<21))>>10;
	int dx=value&0x3FF;

		
	pr_err("isp %s  x=%d y=%d  dx=%d value=%d     \n", __func__ ,x,y,dx,value);


	if(x>0 || y>0)
	{
	touchafaex=x;
	touchafaey=y;
	touchdafaedx=dx;
	pr_err("touch af-ae");
	
    M7MO_AFAE_Set_Touch_Window();


	}
	return rc;
}



int ispcam_set_caf_focusemode( struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	switch (value)
	{
	case 0:
	pr_err("isp   %s caf off  value:  %d\n", __func__,value);
	break;

	case 1:
	pr_err("isp   %s  caf on  value:  %d\n", __func__,value);
	break;

	default:
	pr_err("%s  invalide value is not supported \n",__func__);	
	
	
	}
	return rc;
}


int ispcam_set_focusemode( struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	
	switch (value)
	{
	case 0:
	pr_err("isp   %s normal  value:  %d\n", __func__,value);
	if(caf_state)
	{
		M7MO_CAF_Stop();

	}
	caf_state=0;

	//Set AF range mode normal 

	M7MO_WriteOneByteCRAM(0x0A, 0x2C, 0x00);
	break;

	case 1:
	pr_err("isp   %s  macro value:  %d\n", __func__,value);
	//m7mo_set_focusemode_macro();
	break;

	
	case 2:
		pr_err("isp   %s auto  value:  %d\n", __func__,value);
		//Set AF range mode  normal / infinity 
		M7MO_WriteOneByteCRAM(0x0A, 0x2C, 0x02);
	break;


	case 3:
		pr_err("isp   %s  continuous-picture Continuous Auto Focus  value:  %d\n", __func__,value);
		M7MO_CAF_Start();
		caf_state=1;
	break;


	case 4:
		//Set AF range mode  normal / infinity 
		M7MO_WriteOneByteCRAM(0x0A, 0x2C, 0x00);
		pr_err("isp   %s  infinity value:  %d\n", __func__,value);
	break;
	}
	
	return rc;
}


int ispcam_set_hdr(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
		//int16_t  tmp_reg = 0;
		pr_err("isp   %s   value:  %d\n", __func__,value);

	switch (value)
	{
	case 0:
		 pr_err("%s   hdr off \n",__func__);
       

		M7MO_WriteOneByteCRAM(0x0B, 0x38, 0x00);		
		
		break;

	case 1:
		 pr_err("%s   hdr on \n",__func__);
        
		//M7MO_WriteOneByteCRAM(0x0B, 0x2c, 0x01);
		//M7MO_WriteOneByteCRAM(0x0B, 0x2d, 0x02);
		break;

	case 2:
		 pr_err("%s   hdr auto \n",__func__);
        
		M7MO_WriteOneByteCRAM(0x0B, 0x38, 0x01); 
		M7MO_WriteOneByteCRAM(0x0B, 0x39, 0x02); 
		break;
		
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}

int ispcam_set_facedetect(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
		//int16_t  tmp_reg = 0;
		pr_err("isp   %s   value:  %d\n", __func__,value);

	switch (value)
	{
	case 0:
		 pr_err("%s   fd 0 \n",__func__);
       
#if 0
		M7MO_WriteOneByteCRAM(0x0A, 0x40, 0x00);		
		// Disable face detection (Category 9 byte 0 = 0x10)
		// FD_UPDATE_SELECT = 0	: Auto update frame
		// FD_DRAW_FRAME 	= 1	: Enable frame
		// FACE_DETECT_EN	= 0 : Disable face detection
		M7MO_WriteOneByteCRAM(0x09, 0x00, 0x10);	
#endif
		break;

	case 1:
		 pr_err("%s   fd 1 \n",__func__);
#if 0        
		M7MO_WriteOneByteCRAM(0x09, 0x00, 0x11);		
	
		// Set face detection window as AF window (Category A byte 3 = 0x03)
		// AF_WINDOW		= 3 : Use face detection window
		M7MO_WriteOneByteCRAM(0x0A, 0x40, 0x03);
#endif
		break;
	default:	
	pr_err("%s  invalide value is not supported \n",__func__);
		
	}
	return rc;


}


int ispcam_set_brightness(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	//int16_t  tmp_reg = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);

	switch (value)
	{
	case MSM_V4L2_BRIGHTNESS_L0:
		 pr_err("%s   MSM_V4L2_BRIGHTNESS_L0 \n",__func__);
        

		break;

	case MSM_V4L2_BRIGHTNESS_L1:
		 pr_err("%s   MSM_V4L2_BRIGHTNESS_L1 \n",__func__);
        

		break;

	case MSM_V4L2_BRIGHTNESS_L2:
		 pr_err("%s   MSM_V4L2_BRIGHTNESS_L2 \n",__func__);
        

		break;

	case MSM_V4L2_BRIGHTNESS_L3:
	            pr_err("%s   MSM_V4L2_BRIGHTNESS_L3 \n",__func__);
        

		break;

	case MSM_V4L2_BRIGHTNESS_L4:
		pr_err("%s   MSM_V4L2_BRIGHTNESS_L4 \n",__func__);
		
        

		break;

	case MSM_V4L2_BRIGHTNESS_L5:
		pr_err("%s   MSM_V4L2_BRIGHTNESS_L5 \n",__func__);
        

		break;

	case MSM_V4L2_BRIGHTNESS_L6:
		pr_err("%s   MSM_V4L2_BRIGHTNESS_L6 \n",__func__);
        

		break;

	default:
		pr_err("%s  brghtness value is not supported \n",__func__);
		break;
	}

	return rc;
}


int ispcam_set_iso(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);
	switch (value)
    {
	    case MSM_V4L2_ISO_AUTO:
			pr_err("MSM_V4L2_ISO_AUTO");
			M7MO_WriteOneByteCRAM(0x03, 0x05, 0x00);
		break;
		
	    case MSM_V4L2_ISO_DEBLUR:
			pr_err("MSM_V4L2_ISO_DEBLUR");
			M7MO_WriteOneByteCRAM(0x03, 0x05, 0x00);
		break;
		
        case MSM_V4L2_ISO_100:
			pr_err("MSM_V4L2_ISO_100");
			M7MO_WriteOneByteCRAM(0x03, 0x05, 0x02);
		break;
		
		case MSM_V4L2_ISO_200:
			pr_err("MSM_V4L2_ISO_200");
			M7MO_WriteOneByteCRAM(0x03, 0x05, 0x03);
		break;

		case MSM_V4L2_ISO_400:
			pr_err("MSM_V4L2_ISO_400");
			M7MO_WriteOneByteCRAM(0x03, 0x05, 0x04);
		break;
		
		case MSM_V4L2_ISO_800:
			pr_err("MSM_V4L2_ISO_800");
			M7MO_WriteOneByteCRAM(0x03, 0x05, 0x05);
		break;
		
		case MSM_V4L2_ISO_1600:
			pr_err("MSM_V4L2_ISO_1600");
			M7MO_WriteOneByteCRAM(0x03, 0x05, 0x06);
		break;
		
		

		
		
		default:
			pr_err("invalide level !!");
	}

	return rc;
		
}


int ispcam_set_scenemode(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);
	switch (value)
    {
	    case 0:
			pr_err("CAMERA_BESTSHOT_OFF");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x00);
		break;
		
	    case 1:
			pr_err("CAMERA_BESTSHOT_AUTO");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x01);
		break;
		
        case 2:
			pr_err("CAMERA_BESTSHOT_LANDSCAPE");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x04);
		break;
		
		case 3:
			pr_err("CAMERA_BESTSHOT_SNOW");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x09);
		break;

		case 4:
			pr_err("CAMERA_BESTSHOT_BEACH");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x08);
		break;
		
		case 5:
			pr_err("CAMERA_BESTSHOT_SUNSET");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x0A);
		break;
		
		case 6:
			pr_err("CAMERA_BESTSHOT_NIGHT");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x05);
		break;
		
		case 7:
			pr_err("CAMERA_BESTSHOT_PORTRAIT");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x03);
		break;
		
		case 8:
			pr_err("CAMERA_BESTSHOT_BACKLIGHT");
		break;
		case 9:
			pr_err("CAMERA_BESTSHOT_SPORTS");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x15);
		break;
		case 10:
			pr_err("CAMERA_BESTSHOT_ANTISHAKE steadyphoto");
		break;
		case 11:
			pr_err("CAMERA_BESTSHOT_FLOWERS");
		break;
		case 12:
			pr_err("CAMERA_BESTSHOT_CANDLELIGHT");
		break;
		case 13:
			pr_err("CAMERA_BESTSHOT_FIREWORKS");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x0B);
		break;
		case 14:
			pr_err("CAMERA_BESTSHOT_PARTY");
		break;
		case 15:
			pr_err("CAMERA_BESTSHOT_NIGHT_PORTRAIT");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x06);
		break;
		case 16:
			pr_err("CAMERA_BESTSHOT_THEATRE");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x07);
		break;
		case 17:
			pr_err("CAMERA_BESTSHOT_ACTION");
			M7MO_WriteOneByteCRAM(0x02, 0x59, 0x02);
		break;

		case 18:
			pr_err("CAMERA_BESTSHOT_AR");
		break;
		
		default:
			pr_err("invalide level !!");
	}

	return rc;
		
}


int ispcam_set_contrast(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);
	switch (value)
    {
	    case CAMERA_CONTRAST_LV0:
			pr_err("CAMERA_CONTRAST_LV0");
			M7MO_WriteOneByteCRAM(0x02, 0x25, 0x00);
		break;
		
	    case CAMERA_CONTRAST_LV1:
			pr_err("CAMERA_CONTRAST_LV1");
			M7MO_WriteOneByteCRAM(0x02, 0x25, 0x03);
		break;
		
        case CAMERA_CONTRAST_LV2:
			pr_err("CAMERA_CONTRAST_LV2");
			M7MO_WriteOneByteCRAM(0x02, 0x25, 0x05);
		break;
		
		case CAMERA_CONTRAST_LV3:
			pr_err("CAMERA_CONTRAST_LV3");
			M7MO_WriteOneByteCRAM(0x02, 0x25, 0x07);
		break;

		case CAMERA_CONTRAST_LV4:
			pr_err("CAMERA_CONTRAST_LV4");
			M7MO_WriteOneByteCRAM(0x02, 0x25, 0x09);
		break;
		
		case CAMERA_CONTRAST_LV5:
			pr_err("CAMERA_CONTRAST_LV5");
			M7MO_WriteOneByteCRAM(0x02, 0x25, 0x0B);
		break;
		
		case CAMERA_CONTRAST_LV6:
			pr_err("CAMERA_CONTRAST_LV6");
			M7MO_WriteOneByteCRAM(0x02, 0x25, 0x0C);
		break;
		
		case CAMERA_CONTRAST_LV7:
			pr_err("CAMERA_CONTRAST_LV7");
		break;
		
		case CAMERA_CONTRAST_LV8:
			pr_err("CAMERA_CONTRAST_LV8");
		break;
		default:
			pr_err("invalide level !!");
		
	}

	return rc;
		
}


int ispcam_set_exposure_Compensation(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;

	pr_err("isp   %s   value:  %d\n", __func__,value);

	//temp process for flash 20120608
	pr_err("isp flash_stats =%d isp_cam_state=%d ",flash_stats,isp_cam_state);

	
	switch (value)
	{
	case MSM_V4L2_EXPOSURE_N2:
		 //pr_err("%s   flash on  \n",__func__);
		 
       	M7MO_WriteOneByteCRAM( 0x03, 0x09, 0x00 );	
		 pr_err("%s   MSM_V4L2_EXPOSURE_N2 \n",__func__);

		break;
	case MSM_V4L2_EXPOSURE_N1:
			M7MO_WriteOneByteCRAM( 0x03, 0x09, 0x0A );
		 pr_err("%s   MSM_V4L2_EXPOSURE_N1 \n",__func__);
        

		break;
	case MSM_V4L2_EXPOSURE_D:
		M7MO_WriteOneByteCRAM( 0x03, 0x09, 0x1E);
		 pr_err("%s   MSM_V4L2_EXPOSURE_D 00 \n",__func__);
        

		break;
	case MSM_V4L2_EXPOSURE_P1:
		M7MO_WriteOneByteCRAM( 0x03, 0x09, 0x28);
		 pr_err("%s   MSM_V4L2_EXPOSURE_P1 \n",__func__);
        

		break;
	case MSM_V4L2_EXPOSURE_P2:
		pr_err("%s   MSM_V4L2_EXPOSURE_P2 \n",__func__);
        //pr_err("%s   flash off  \n",__func__);
       	M7MO_WriteOneByteCRAM( 0x03, 0x09, 0x3C );	
		break;

		default:
			pr_err("invalide level !!");
			
	}

	
		
	
	
	return rc;

}
int ispcam_set_effect(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);
	switch (value)
	{
	case CAMERA_EFFECT_OFF:
		 pr_err("%s   CAMERA_EFFECT_OFF 0\n",__func__);
        M7MO_WriteOneByteCRAM(0x02, 0x0B, 0x00);

		break;
	case CAMERA_EFFECT_MONO:
		 pr_err("%s   CAMERA_EFFECT_MONO 1\n",__func__);
        
		M7MO_WriteOneByteCRAM(0x02, 0x0B, 0x01);	
		M7MO_WriteOneByteCRAM(0x02, 0x09, 0x00);
		M7MO_WriteOneByteCRAM(0x02, 0x0A, 0x00);

		break;
	case CAMERA_EFFECT_NEGATIVE:
		 pr_err("%s   CAMERA_EFFECT_NEGATIVE 2\n",__func__);
        

		break;
	case CAMERA_EFFECT_SOLARIZE:
		 pr_err("%s   CAMERA_EFFECT_SOLARIZE 3\n",__func__);
        

		break;
	case CAMERA_EFFECT_SEPIA:
		 pr_err("%s   CAMERA_EFFECT_SEPIA 4\n",__func__);
        
		M7MO_WriteOneByteCRAM(0x02, 0x0B, 0x01);	
		M7MO_WriteOneByteCRAM(0x02, 0x09, 0xD8);
		M7MO_WriteOneByteCRAM(0x02, 0x0A, 0x18);

		break;
	case CAMERA_EFFECT_POSTERIZE:
		 pr_err("%s   CAMERA_EFFECT_POSTERIZE 5\n",__func__);
        

		break;
	case CAMERA_EFFECT_WHITEBOARD:
		 pr_err("%s   CAMERA_EFFECT_WHITEBOARD 6\n",__func__);
        

		break;
	case CAMERA_EFFECT_BLACKBOARD:
		 pr_err("%s   CAMERA_EFFECT_BLACKBOARD 7\n",__func__);
        

		break;

	case CAMERA_EFFECT_AQUA:
		 pr_err("%s   CAMERA_EFFECT_AQUA 8\n",__func__);

		break;
		
	case CAMERA_EFFECT_EMBOSS:
		 pr_err("%s   CAMERA_EFFECT_EMBOSS 9\n",__func__);
        break;
		
	case CAMERA_EFFECT_SKETCH:
	 	pr_err("%s   CAMERA_EFFECT_SKETCH 10\n",__func__);
        break;
		
	 case CAMERA_EFFECT_NEON:
	 	pr_err("%s   CAMERA_EFFECT_NEON 11\n",__func__);
		//M7MO_WriteOneByteCRAM(0x02, 0x0B, 0x01);	
		//M7MO_WriteOneByteCRAM(0x02, 0x09, 0xD8);
		//M7MO_WriteOneByteCRAM(0x02, 0x0A, 0x18);
        break;
		
	 case CAMERA_EFFECT_MAX:
	 	pr_err("%s   CAMERA_EFFECT_MAX \n",__func__);
		break; 

	

	default:
			pr_err("invalide level !!");

		}
		
		
		
	return rc;

}






int ispcam_set_wb(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("isp   %s   value:  %d\n", __func__,value);
	switch (value)
	{
	case MSM_V4L2_WB_MIN_MINUS_1:
		 pr_err("%s   MSM_V4L2_WB_MIN_MINUS_1 0 \n",__func__);
        

		break;
	case MSM_V4L2_WB_AUTO:
		 pr_err("%s   MSM_V4L2_WB_AUTO  1\n",__func__);
        
		M7MO_WriteOneByteCRAM(0x06, 0x02, 0x01);

		break;
	case MSM_V4L2_WB_CUSTOM:
		 pr_err("%s   MSM_V4L2_WB_CUSTOM 2 \n",__func__);
        

		break;
	case MSM_V4L2_WB_INCANDESCENT:
		 pr_err("%s   MSM_V4L2_WB_INCANDESCENT 3 \n",__func__);
        
		M7MO_WriteOneByteCRAM(0x06, 0x02, 0x02);
		M7MO_WriteOneByteCRAM(0x06, 0x03, 0x01);

		break;
	case MSM_V4L2_WB_FLUORESCENT:
		 pr_err("%s   MSM_V4L2_WB_FLUORESCENT 4 \n",__func__);
        
		M7MO_WriteOneByteCRAM(0x06, 0x02, 0x02);
		M7MO_WriteOneByteCRAM(0x06, 0x03, 0x02);

		break;
	case 5:
		 pr_err("%s   MSM_V4L2_WB_DAYLIGHT 5 \n",__func__);
        M7MO_WriteOneByteCRAM(0x06, 0x02, 0x02);
		M7MO_WriteOneByteCRAM(0x06, 0x03, 0x04);

		break;
	case MSM_V4L2_WB_CLOUDY_DAYLIGHT :
		 pr_err("%s   MSM_V4L2_WB_CLOUDY_DAYLIGHT 6 \n",__func__);
        M7MO_WriteOneByteCRAM(0x06, 0x02, 0x02);
		M7MO_WriteOneByteCRAM(0x06, 0x03, 0x05);

		break;
	case MSM_V4L2_WB_SHADE:
		 pr_err("%s   MSM_V4L2_WB_SHADE  7\n",__func__);
        

		break;

	case MSM_V4L2_WB_OFF:
		 pr_err("%s   MSM_V4L2_WB_OFF \n",__func__);
        

		break;

	default:
			pr_err("invalide level !!");	

		}
		
		
		
	return rc;

}

struct msm_sensor_v4l2_ctrl_info_t ispcam_v4l2_ctrl_info[] = {

	{
		.ctrl_id = V4L2_CID_SHARPNESS,
		.min = 0,
		.max = 30,
		.step = 5,
		.s_v4l2_ctrl = ispcam_set_sharpness,
	},

	{
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N2,
		.max = MSM_V4L2_EXPOSURE_P2,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_exposure_Compensation,
	},

	{
		.ctrl_id = V4L2_CID_AUTO_WHITE_BALANCE,
		.min = MSM_V4L2_WB_MIN_MINUS_1,
		.max = MSM_V4L2_WB_OFF,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_wb,
	},

	{
		.ctrl_id = V4L2_CID_BRIGHTNESS,
		.min = MSM_V4L2_BRIGHTNESS_L0,
		.max = MSM_V4L2_BRIGHTNESS_L6,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_brightness,
	},
	{
		.ctrl_id = V4L2_CID_COLORFX,
		.min = V4L2_COLORFX_NONE,
		.max = V4L2_COLORFX_VIVID,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_effect,
	},
	{
		.ctrl_id = V4L2_CID_CONTRAST,
		.min = CAMERA_CONTRAST_LV0,
		.max = CAMERA_CONTRAST_LV9,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_contrast,
	},
	{
		.ctrl_id = V4L2_CID_BESTSHOT_MODE,
		.min = 0,
		.max = 18,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_scenemode,
	},

	{
		.ctrl_id = V4L2_CID_ISO,
		.min = MSM_V4L2_ISO_AUTO,
		.max = MSM_V4L2_ISO_1600,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_iso,
	},
	{
		.ctrl_id = V4L2_CID_FACEDETECT_MODE,
		.min = 0,
		.max = 1,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_facedetect,
	},

	{
		.ctrl_id = V4L2_CID_HDR,
		.min = 0,
		.max = 2,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_hdr,
	},
	{
		.ctrl_id = V4L2_CID_FOCUSEMODE,
		.min = 0,
		.max = 5,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_focusemode,
	},
	{
		.ctrl_id = V4L2_CID_CAF,
		.min = 0,
		.max = 1,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_caf_focusemode,
	},
	{
		.ctrl_id = V4L2_CID_TOUCH_AF_AE,
		.min = 0,
		.max = 1,
		.step = 1,
		.s_v4l2_ctrl_e = ispcam_set_touch_af_ae,
	},
	{
		.ctrl_id = V4L2_CID_EXPOSURE_METR,
		.min = 0,
		.max = 2,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_exposure ,
	},
	{
		.ctrl_id = V4L2_CID_REDEYE_REDUCTION,
		.min = 0,
		.max = 1,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_redeye_reduce,
	},
	{
		.ctrl_id = V4L2_CID_ANTIBANDING,
		.min = 0,
		.max = 3,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_antibanding,
	},

	{
		.ctrl_id = V4L2_CID_SATURATION,
		.min = 0,
		.max = 6,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_saturation,
	},

	{
		.ctrl_id = V4L2_CID_FLASH_LED_TORCH,
		.min = 0,
		.max = 3,
		.step = 1,
		.s_v4l2_ctrl = ispcam_set_flash_led_torch,
	},

		



};


void ispcam_msm_sensor_start_stream(struct msm_sensor_ctrl_t *s_ctrl)
{//no need

pr_err("%s:   X    %d\n", __func__, __LINE__);

}

void ispcam_msm_sensor_stop_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
 //no need
pr_err("%s:   X    %d\n", __func__, __LINE__);

}


void ispcam_msm_sensor_group_hold_on(struct msm_sensor_ctrl_t *s_ctrl)
{ //no need
	pr_err("%s:   X    %d\n", __func__, __LINE__);
}

void ispcam_msm_sensor_group_hold_off(struct msm_sensor_ctrl_t *s_ctrl)
{//no need
	pr_err("%s:   X    %d\n", __func__, __LINE__);

}

int32_t ispcam_msm_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
						struct fps_cfg *fps)
{
	//uint16_t total_lines_per_frame; //no need
	int32_t rc = 0;
	s_ctrl->fps_divider = fps->fps_div;

	if (s_ctrl->curr_res != MSM_SENSOR_INVALID_RES) {
		
	}
	pr_err("%s:   X    %d\n", __func__, __LINE__);
	
	return rc;
}


int32_t ispcam_af_trigger(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc=0;

	pr_err("%s:   X   touchafae_state= %d\n", __func__, touchafae_state);

	M7MO_AF_Start(0x03);
/*
    if(touchafae_state == 0)
	{
		M7MO_AF_Start(0x03);
    }
	else
	{
     pr_err("touchafae_state =1 .so skip execut M7MO_AF_Start(E_M7MO_AF_MODE_NORMAL_AF_FAST) ");
	}
*/
	
	pr_err("%s:   X    %d\n", __func__, __LINE__);
	return rc;
}

int32_t isp_cam_matchid(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc=0;


	pr_err("%s:   X    %d\n", __func__, __LINE__);
	return rc;
}



int32_t ispcam_msm_sensor_set_sensor_mode(struct msm_sensor_ctrl_t *s_ctrl,
	int mode, int res)
{
//use
	int32_t rc = 0;
	pr_err("%s:   X    %d\n", __func__, __LINE__);
	if (s_ctrl->curr_res != res) {
		s_ctrl->curr_frame_length_lines =
			s_ctrl->msm_sensor_reg->
			output_settings[res].frame_length_lines;

		s_ctrl->curr_line_length_pclk =
			s_ctrl->msm_sensor_reg->
			output_settings[res].line_length_pclk;

		if (s_ctrl->func_tbl->sensor_setting
			(s_ctrl, MSM_SENSOR_UPDATE_PERIODIC, res) < 0)
			return rc;
		s_ctrl->curr_res = res;
	}

	return rc;
}


int32_t ispcam_msm_sensor_get_output_info(struct msm_sensor_ctrl_t *s_ctrl,
		struct sensor_output_info_t *sensor_output_info)
{//use
	int rc = 0;
	pr_err("%s:   X    %d\n", __func__, __LINE__);
	
	sensor_output_info->num_info = s_ctrl->msm_sensor_reg->num_conf;
	if (copy_to_user((void *)sensor_output_info->output_info,
		s_ctrl->msm_sensor_reg->output_settings,
		sizeof(struct msm_sensor_output_info_t) *
		s_ctrl->msm_sensor_reg->num_conf))
		rc = -EFAULT;

	return rc;
}


int32_t ispcam_msm_sensor_mode_init(struct msm_sensor_ctrl_t *s_ctrl,
			int mode, struct sensor_init_cfg *init_info)
{
//use 
	int32_t rc = 0;
	s_ctrl->fps_divider = Q10;
	s_ctrl->cam_mode = MSM_SENSOR_MODE_INVALID;

	pr_err("%s: %d\n", __func__, __LINE__);
	if (mode != s_ctrl->cam_mode) {
		s_ctrl->curr_res = MSM_SENSOR_INVALID_RES;
		s_ctrl->cam_mode = mode;

		rc = s_ctrl->func_tbl->sensor_setting(s_ctrl,
			MSM_SENSOR_REG_INIT, 0);
	}
	return rc;
}

int32_t ispcam_msm_sensor_config(struct msm_sensor_ctrl_t *s_ctrl, void __user *argp)
{ // use
	struct sensor_cfg_data cdata;
	long   rc = 0;

	pr_err("%s:   X    %d\n", __func__, __LINE__);

	
	
	if (copy_from_user(&cdata,
		(void *)argp,
		sizeof(struct sensor_cfg_data)))
		return -EFAULT;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	pr_err("msm_sensor_config: cfgtype = %d\n",
	cdata.cfgtype);
		switch (cdata.cfgtype) {
		case CFG_SET_FPS:
		case CFG_SET_PICT_FPS:
			if (s_ctrl->func_tbl->
			sensor_set_fps == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->
				sensor_set_fps(
				s_ctrl,
				&(cdata.cfg.fps));
			break;

		case CFG_SET_EXP_GAIN:
			if (s_ctrl->func_tbl->
			sensor_write_exp_gain == NULL) {
				rc = -EFAULT;
				break;
			}
			rc =
				s_ctrl->func_tbl->
				sensor_write_exp_gain(
					s_ctrl,
					cdata.cfg.exp_gain.gain,
					cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_PICT_EXP_GAIN:
			if (s_ctrl->func_tbl->
			sensor_write_snapshot_exp_gain == NULL) {
				rc = -EFAULT;
				break;
			}
			rc =
				s_ctrl->func_tbl->
				sensor_write_snapshot_exp_gain(
					s_ctrl,
					cdata.cfg.exp_gain.gain,
					cdata.cfg.exp_gain.line);
			break;

		case CFG_SET_MODE:
			if (s_ctrl->func_tbl->
			sensor_set_sensor_mode == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->
				sensor_set_sensor_mode(
					s_ctrl,
					cdata.mode,
					cdata.rs);
			break;

		case CFG_SET_EFFECT:
			break;

		case CFG_SENSOR_INIT:
			if (s_ctrl->func_tbl->
			sensor_mode_init == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->
				sensor_mode_init(
				s_ctrl,
				cdata.mode,
				&(cdata.cfg.init_info));
			break;

		case CFG_GET_OUTPUT_INFO:
			if (s_ctrl->func_tbl->
			sensor_get_output_info == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->
				sensor_get_output_info(
				s_ctrl,
				&cdata.cfg.output_info);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_GET_EEPROM_DATA:
			if (s_ctrl->sensor_eeprom_client == NULL ||
				s_ctrl->sensor_eeprom_client->
				func_tbl.eeprom_get_data == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->sensor_eeprom_client->
				func_tbl.eeprom_get_data(
				s_ctrl->sensor_eeprom_client,
				&cdata.cfg.eeprom_data);

			if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_eeprom_data_t)))
				rc = -EFAULT;
			break;
		/*added by CDZ_CAM_ZTE for AF of yuv sensor*/
		case CFG_SET_AF:
            		if (s_ctrl->func_tbl->af_trigger == NULL) {
				rc = -EFAULT;
				break;
			}
			rc = s_ctrl->func_tbl->af_trigger(s_ctrl);
			break;
            	default:
			rc = -EFAULT;
			break;
		}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}




static int __init msm_sensor_init_module(void)
{
	pr_err("isp %s: %d\n", __func__, __LINE__);

	return i2c_add_driver(&isp_cam_i2c_driver);
}

static struct v4l2_subdev_core_ops isp_cam_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};


static struct v4l2_subdev_video_ops isp_cam_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops isp_cam_subdev_ops = {
	.core = &isp_cam_subdev_core_ops,
	.video  = &isp_cam_subdev_video_ops,
};

static struct msm_sensor_fn_t isp_cam_func_tbl = {
	.sensor_start_stream = ispcam_msm_sensor_start_stream,
	.sensor_stop_stream = ispcam_msm_sensor_stop_stream,
	.sensor_group_hold_on = ispcam_msm_sensor_group_hold_on,
	.sensor_group_hold_off = ispcam_msm_sensor_group_hold_off,
	.sensor_set_fps = ispcam_msm_sensor_set_fps,
	.sensor_write_exp_gain = isp_cam_write_exp_gain,
	.sensor_write_snapshot_exp_gain = isp_cam_write_exp_gain,
	.sensor_setting = msm_isp_cam_sensor_setting,
	.sensor_set_sensor_mode = ispcam_msm_sensor_set_sensor_mode,
	.sensor_mode_init = ispcam_msm_sensor_mode_init,
	.sensor_get_output_info = ispcam_msm_sensor_get_output_info,
	.sensor_config = ispcam_msm_sensor_config,
	//.sensor_open_init = isp_cam_sensor_open_init,
	//.sensor_release = isp_cam_sensor_release,
	.sensor_power_up = msm_isp_cam_sensor_power_up,
	.sensor_power_down = msm_isp_cam_sensor_power_down,
	//.sensor_probe = msm_sensor_probe,
	.af_trigger=ispcam_af_trigger,
	.sensor_match_id=isp_cam_matchid,
};

static struct msm_sensor_reg_t isp_cam_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = isp_cam_start_settings, 
	.start_stream_conf_size = ARRAY_SIZE(isp_cam_start_settings),
	.stop_stream_conf = isp_cam_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(isp_cam_stop_settings),
	.group_hold_on_conf = isp_cam_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(isp_cam_groupon_settings),
	.group_hold_off_conf = isp_cam_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(isp_cam_groupoff_settings),
	.init_settings = &isp_cam_init_conf[0],
	.init_size = ARRAY_SIZE(isp_cam_init_conf),
	.mode_settings = &isp_cam_confs[0],
	.output_settings = &isp_cam_dimensions[0],
	.num_conf = ARRAY_SIZE(isp_cam_confs),
};

static struct msm_sensor_ctrl_t isp_cam_s_ctrl = {
	.msm_sensor_reg = &isp_cam_regs,
	.msm_sensor_v4l2_ctrl_info = ispcam_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(ispcam_v4l2_ctrl_info),
	.sensor_i2c_client = &isp_cam_sensor_i2c_client,
	.sensor_i2c_addr = 0x3E,
	.sensor_output_reg_addr = &isp_cam_reg_addr,
	.sensor_id_info = &isp_cam_id_info,
	.sensor_exp_gain_info = &isp_cam_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csi_params = &isp_cam_csi_params_array[0],
	.msm_sensor_mutex = &isp_cam_mut,
	.sensor_i2c_driver = &isp_cam_i2c_driver,
	.sensor_v4l2_subdev_info = isp_cam_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(isp_cam_subdev_info),
	.sensor_v4l2_subdev_ops = &isp_cam_subdev_ops,
	.func_tbl = &isp_cam_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
	
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("ISP_CAM mbg0410 for sensor driver");
MODULE_LICENSE("GPL v2");


