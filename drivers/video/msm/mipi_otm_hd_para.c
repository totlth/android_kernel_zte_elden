#include "mipi_otm_hd.h"
#include "mdp4.h"
/*color enhance*/
#define COLOR_ENHANCE
extern  struct dsi_buf mipi_tx_buf;
extern struct dsi_buf mipi_rx_buf;
extern struct msm_fb_data_type *local_mfd;
extern void lcd_reset_esd(struct msm_fb_data_type *mfd);
struct workqueue_struct *lcd_esd_queue;
struct work_struct lcd_esd_work;
struct timer_list lcd_esd_timer;
struct workqueue_struct *lcd_esd_queue;
char timer_deleted = 0;


static struct dsi_cmd_desc * panel_init_para = NULL;
static uint32 panel_init_para_len = 0;
static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char OTM_CMI_panelid_rd_para[] = {0xA1, 0x00}; 

static struct dsi_cmd_desc OTM_CMI_panelid_rd_cmd = 
{
	DTYPE_GEN_READ1, 1, 0, 0, 1,sizeof(OTM_CMI_panelid_rd_para), OTM_CMI_panelid_rd_para
};
static char otm_cmd_oa[2] = {0x0a, 0x00};
static char otm_cmd_ob[2] = {0x0b, 0x00};
static struct dsi_cmd_desc get_otm1281_0x0a_value_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(otm_cmd_oa), otm_cmd_oa
};
static struct dsi_cmd_desc get_otm1281_0x0b_value_cmd = {
	DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(otm_cmd_ob), otm_cmd_ob
};



//truly OTM1281A
static char truly_cmd_FF1[] = {0xFF, 0x12,0x80,0x01};
static char truly_shift_FF2[] = {0x00, 0x80};
static char truly_cmd_FF2[] = {0xFF, 0x12,0x80};
static char truly_shift_b0[] = {0x00, 0xb0};
static char truly_cmd_b0[] = {0xb0, 0x20};
static char truly_shift_C51[] = {0x00, 0x90};
static char truly_cmd_C51[] = {0xC5, 0x20,0x6F,0x02,0x88,0x1d,0x15,0x00,0x08,0x44,0x44,0x44};
static char truly_shift_C52[] = {0x00, 0xa0};
static char truly_cmd_C52[] = {0xC5, 0x20,0x6F,0x02,0x88,0x1d,0x15,0x00,0x08};
static char truly_shift_C53[] = {0x00, 0x80};
static char truly_cmd_C53[] = {0xC5, 0x20,0x01,0x00,0xB0,0xB0,0x00,0x08,0x00};
static char truly_shift_D8[] = {0x00, 0x00};
static char truly_cmd_D8[] = {0xD8, 0x58,0x00,0x58,0x00};
static char truly_shift_F5[] = {0x00, 0xb8};
static char truly_cmd_F5[] = {0xF5, 0x0C,0x12};


static char truly_shift_C1_80[] = {0x00, 0x80};
static char truly_cmd_C1_80[] = {0xc1, 0x85};
static char truly_shift_C5_D0[] = {0x00, 0xD0};
static char truly_cmd_C5_D0[] = {0xc5,0x38};

#ifdef COLOR_ENHANCE
static char truly_shift_83[] = {0x00, 0x83};
static char truly_cmd_F4[] = {0xf4, 0x00};
static char truly_shift_00[] = {0x00, 0x00};
static char truly_cmd_D4[] = {0xD4,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,
						    0x40};
static char truly_cmd_D5[] = {0xD5,0x00,0x60,0x00,0x60,0x00,0x5f,0x00,0x5f,0x00,
						  0x5e,0x00,0x5e,0x00,0x5d,0x00,0x5d,0x00,0x5d,0x00,
						  0x5c,0x00,0x5c,0x00,0x5b,0x00,0x5b,0x00,0x5a,0x00,
						  0x5a,0x00,0x5a,0x00,0x5b,0x00,0x5c,0x00,0x5d,0x00,
						  0x5d,0x00,0x5e,0x00,0x5f,0x00,0x60,0x00,0x61,0x00,
						  0x62,0x00,0x63,0x00,0x63,0x00,0x64,0x00,0x65,0x00,
						  0x66,0x00,0x67,0x00,0x68,0x00,0x69,0x00,0x69,0x00,
						  0x6a,0x00,0x6b,0x00,0x6c,0x00,0x6d,0x00,0x6e,0x00,
						  0x6f,0x00,0x6f,0x00,0x70,0x00,0x71,0x00,0x72,0x00,
						  0x73,0x00,0x74,0x00,0x74,0x00,0x75,0x00,0x76,0x00,
						  0x77,0x00,0x78,0x00,0x78,0x00,0x79,0x00,0x7a,0x00,
						  0x7b,0x00,0x7c,0x00,0x7d,0x00,0x7d,0x00,0x7e,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7f,0x00,
						  0x7f,0x00,0x7f,0x00,0x7f,0x00,0x7e,0x00,0x7d,0x00,
						  0x7c,0x00,0x7b,0x00,0x7a,0x00,0x7a,0x00,0x79,0x00,
						  0x78,0x00,0x77,0x00,0x76,0x00,0x76,0x00,0x75,0x00,
						  0x74,0x00,0x73,0x00,0x72,0x00,0x71,0x00,0x71,0x00,
						  0x70,0x00,0x6f,0x00,0x6e,0x00,0x6d,0x00,0x6c,0x00,
						  0x6c,0x00,0x6b,0x00,0x6a,0x00,0x69,0x00,0x68,0x00,
						  0x67,0x00,0x66,0x00,0x66,0x00,0x66,0x00,0x65,0x00,
						  0x65,0x00,0x64,0x00,0x64,0x00,0x63,0x00,0x63,0x00,
						  0x63,0x00,0x62,0x00,0x62,0x00,0x61,0x00,0x61,0x00,
						  0x60};
#endif
static char truly_cmd_36[] = {0x36, 0xd0};
static char truly_cmd_35[] = {0x35, 0x00};

static char truly_cmd_2c[] = {0x2c, 0x00};
static char truly_cmd_3c[] = {0x3c, 0x00};
static char truly_shift_B0_80[] = {0x00,0x80};
static char truly_cmd_B0_80[] = {0xb0,0x60};
static char truly_shift_B0_B1[] = {0x00,0xB1};
static char truly_cmd_B0_B1[] = {0xb0,004};




static struct dsi_cmd_desc OTM1281_AUO_CMD_INIT_PARA[] = {
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_FF1),truly_cmd_FF1},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_FF2),truly_shift_FF2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_FF2),truly_cmd_FF2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_b0),truly_shift_b0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_b0),truly_cmd_b0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C51),truly_shift_C51},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C51),truly_cmd_C51},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C52),truly_shift_C52},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C52),truly_cmd_C52},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C53),truly_shift_C53},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C53),truly_cmd_C53},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_D8),truly_shift_D8},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D8),truly_cmd_D8},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_F5),truly_shift_F5},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F5),truly_cmd_F5},
/*	
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C1_80),truly_shift_C1_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C1_80),truly_cmd_C1_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C5_D0),truly_shift_C5_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C5_D0),truly_cmd_C5_D0},
*/

#ifdef COLOR_ENHANCE
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_83),truly_shift_83},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F4),truly_cmd_F4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D4),truly_cmd_D4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 1, sizeof(truly_cmd_D5),truly_cmd_D5},

#endif
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(truly_cmd_36),truly_cmd_36},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_35),truly_cmd_35},
		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep),exit_sleep},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on},

		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_2c),truly_cmd_2c},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_3c), truly_cmd_3c},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_shift_B0_80),truly_shift_B0_80},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_B0_80), truly_cmd_B0_80},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_shift_B0_B1),truly_shift_B0_B1},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_B0_B1), truly_cmd_B0_B1},

};



static struct dsi_cmd_desc OTM1281_AUO_VIDEO_INIT_PARA[] = {
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_FF1),truly_cmd_FF1},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_FF2),truly_shift_FF2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_FF2),truly_cmd_FF2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_b0),truly_shift_b0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_b0),truly_cmd_b0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C51),truly_shift_C51},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C51),truly_cmd_C51},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C52),truly_shift_C52},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C52),truly_cmd_C52},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C53),truly_shift_C53},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C53),truly_cmd_C53},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_D8),truly_shift_D8},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D8),truly_cmd_D8},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_F5),truly_shift_F5},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F5),truly_cmd_F5},
	
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C1_80),truly_shift_C1_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C1_80),truly_cmd_C1_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_C5_D0),truly_shift_C5_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_C5_D0),truly_cmd_C5_D0},


#ifdef COLOR_ENHANCE
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_83),truly_shift_83},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F4),truly_cmd_F4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D4),truly_cmd_D4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D5),truly_cmd_D5},

#endif
		{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(truly_cmd_36),truly_cmd_36},
//		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_35),truly_cmd_35},
		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep),exit_sleep},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on},

		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_2c),truly_cmd_2c},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_3c), truly_cmd_3c},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_shift_B0_80),truly_shift_B0_80},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_B0_80), truly_cmd_B0_80},
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_shift_B0_B1),truly_shift_B0_B1},
	
		{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(truly_cmd_B0_B1), truly_cmd_B0_B1},

};


//< 2012/6/1-N9500_add_new_lcd_bootloader_kernel-lizhiye- < short commond here >
//cmi OTM1280A
static char cmi_cmd_FF[] = {0xFF, 0x12,0x80,0x01};
static char cmi_shift_FF_80[] = {0x00, 0x80};
static char cmi_cmd_FF_80[] = {0xFF, 0x12,0x80};
static char cmi_shift_B3_A0[] = {0x00,0xa0};
static char cmi_cmd_B3_A0[] = {0xb3,0x38,0x38};

static char cmi_cmd_truly_shift_b0[] = {0x00, 0xb0}; 
static char cmi_cmd_truly_cmd_b0[] = {0xb0, 0x20};

static char cmi_shift_CB_80[] = {0x00,0x80};
static char cmi_cmd_CB_80[] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_90[] = {0x00,0x90};
static char cmi_cmd_CB_90[] = {0xcb,0x00,0xc0,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_A0[] = {0x00,0xa0};
static char cmi_cmd_CB_A0[] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_B0[] = {0x00,0xb0};
static char cmi_cmd_CB_B0[] = {0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_C0[] = {0x00,0xc0};
static char cmi_cmd_CB_C0[] = {0xcb,0x04,0x00,0x0f,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0xf4};
static char cmi_shift_CB_D0[] = {0x00,0xd0};
static char cmi_cmd_CB_D0[] = {0xcb,0xf4,0xf4,0x00,0xf4,0x08,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_CB_E0[] = {0x00,0xe0};
static char cmi_cmd_CB_E0[] = {0xcb,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
static char cmi_shift_CB_F0[] = {0x00,0xf0};
static char cmi_cmd_CB_F0[] = {0xcb,0x00,0x70,0x01,0x00,0x00};
static char cmi_shift_CC_80[] = {0x00,0x80};
static char cmi_cmd_CC_80[] = {0xcc,0x41,0x42,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x52,0x55,0x43,0x53,0x65,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40};
static char cmi_shift_CC_A0[] = {0x00,0xA0};
static char cmi_cmd_CC_A0[] = {0xcc,0x41,0x42,0x47,0x48,0x4C,0x4B,0x4A,0x49,0x52,0x55,0x43,0x53,0x65,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0x01};
static char cmi_shift_CC_C0[] = {0x00,0xC0};
static char cmi_cmd_CC_C0[] = {0xcc,0x41,0x42,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x52,0x55,0x43,0x53,0x54,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40};
static char cmi_shift_CC_E0[] = {0x00,0xE0};
static char cmi_cmd_CC_E0[] = {0xcc,0x41,0x42,0x47,0x48,0x4C,0x4B,0x4A,0x49,0x52,0x55,0x43,0x53,0x54,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0x01};
static char cmi_shift_C1_90[] = {0x00,0x90};
static char cmi_cmd_C1_90[] = {0xc1,0x22,0x00,0x00,0x00,0x00};
static char cmi_shift_C0_80[] = {0x00,0x80};
static char cmi_cmd_C0_80[] = {0xc0,0x00,0x87,0x00,0x06,0x0a,0x00,0x87,0x06,0x0a,0x00,0x00,0x00};
static char cmi_shift_C0_90[] = {0x00,0x90};
static char cmi_cmd_C0_90[] = {0xc0,0x00,0x0a,0x00,0x14,0x00,0x2a};
static char cmi_shift_C0_A0[] = {0x00,0xA0};
static char cmi_cmd_C0_A0[] = {0xc0,0x00,0x03,0x01,0x01,0x01,0x01,0x1a,0x03,0x00,0x02};
static char cmi_shift_C2_80[] = {0x00,0x80};
static char cmi_cmd_C2_80[] = {0xc2,0x03,0x02,0x00,0x00,0x00,0x02,0x00,0x22};
static char cmi_shift_C2_90[] = {0x00,0x90};
static char cmi_cmd_C2_90[] = {0xc2,0x03,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x22};
static char cmi_shift_C2_A0[] = {0x00,0xA0};
static char cmi_cmd_C2_A0[] = {0xc2,0xff,0x00,0xff,0x00,0x00,0x0a,0x00,0x0a};
static char cmi_shift_C2_B0[] = {0x00,0xB0};
static char cmi_cmd_C2_B0[] = {0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_C2_C0[] = {0x00,0xC0};
static char cmi_cmd_C2_C0[] = {0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char cmi_shift_C2_E0[] = {0x00,0xE0};
static char cmi_cmd_C2_E0[] = {0xc2,0x84,0x00,0x10,0x0d};
static char cmi_shift_C2_B3[] = {0x00,0xB3};
static char cmi_cmd_C2_B3[] = {0xc0,0x0f};
static char cmi_shift_C1_A2[] = {0x00,0xA2};
static char cmi_cmd_C1_A2[] = {0xc1,0xff};
static char cmi_shift_C0_B4[] = {0x00,0xB4};
static char cmi_cmd_C0_B4[] = {0xc0,0x54,0x00};
static char cmi_shift_C5_80[] = {0x00,0x80};
static char cmi_cmd_C5_80[] = {0xc5,0x20,0x07,0x00,0xb0,0xb0,0x00,0x08,0x00};
static char cmi_shift_C5_90[] = {0x00,0x90};
static char cmi_cmd_C5_90[] = {0xc5,0x20,0x85,0x02,0x88,0x96,0x15,0x00,0x08,0x44,0x44,0x44};

static char cmi_shift_D8_00[] = {0x00,0x00};
static char cmi_cmd_D8_00[] = {0xd8,0x52,0x00,0x52,0x00};
static char cmi_shift_D9_00[] = {0x00,0x00};
static char cmi_cmd_D9_00[] = {0xd9,0x8f,0x73,0x80};
static char cmi_shift_C0_C0[] = {0x00,0xC0};
static char cmi_cmd_C0_C0[] = {0xc0,0x95};
static char cmi_shift_C0_D0[] = {0x00,0xD0};
static char cmi_cmd_C0_D0[] = {0xc0,0x05};
static char cmi_shift_F5_B6[] = {0x00,0xB6};
static char cmi_cmd_F5_B6[] = {0xf5,0x00,0x00};
static char cmi_shift_B3_B0[] = {0x00,0xB0};
static char cmi_cmd_B3_B0[] = {0xb3,0x11};
static char cmi_shift_F5_B0[] = {0x00,0xB0};
static char cmi_cmd_F5_B0[] = {0xf5,0x00,0x20};
static char cmi_shift_F5_B8[] = {0x00,0xB8};
static char cmi_cmd_F5_B8[] = {0xf5,0x0c,0x12};
static char cmi_shift_F5_94[] = {0x00,0x94};
static char cmi_cmd_F5_94[] = {0xf5,0x0a,0x14,0x06,0x17};
static char cmi_shift_F5_A2[] = {0x00,0xA2};
static char cmi_cmd_F5_A2[] = {0xf5,0x0a,0x14,0x07,0x14};
static char cmi_shift_F5_90[] = {0x00,0x90};
static char cmi_cmd_F5_90[] = {0xf5,0x07,0x16,0x07,0x14};
static char cmi_shift_F5_A0[] = {0x00,0xA0};
static char cmi_cmd_F5_A0[] = {0xf5,0x02,0x12,0x0a,0x12,0x07,0x12,0x06,0x12,0x0b,0x12,0x08,0x12};
static char cmi_cmd_36[] = {0x36,0xd1};
static char cmi_cmd_35[] = {0x35,0x00};
static char cmi_shift_D6_80[] = {0x00,0x80};
static char cmi_cmd_D6_80[] = {0xd6,0x00};
static char cmi_shift_E1_00[] = {0x00,0x00};
static char cmi_cmd_E1_00[] = {0xE1,0x2C,0x2F,0x36,0x3E,0x0B,0x05,0x14,0x09,0x07,0x08,0x09,0x1C,0x05,0x0B,0x11,0x0E,0x0B,0x0B};
static char cmi_shift_E2_00[] = {0x00,0x00};
static char cmi_cmd_E2_00[] = {0xE2,0x2C,0x2F,0x36,0x3E,0x0B,0x05,0x14,0x09,0x07,0x08,0x09,0x1C,0x05,0x0B,0x11,0x0E,0x0B,0x0B};
static char cmi_shift_E3_00[] = {0x00,0x00};
static char cmi_cmd_E3_00[] = {0xE3,0x2C,0x2E,0x35,0x3C,0x0D,0x06,0x16,0x09,0x07,0x08,0x0A,0x1A,0x05,0x0B,0x12,0x0E,0x0B,0x0B};
static char cmi_shift_E4_00[] = {0x00,0x00};
static char cmi_cmd_E4_00[] = {0xE4,0x2C,0x2E,0x35,0x3C,0x0D,0x06,0x16,0x09,0x07,0x08,0x0A,0x1A,0x05,0x0B,0x12,0x0E,0x0B,0x0B};
static char cmi_shift_E5_00[] = {0x00,0x00};
static char cmi_cmd_E5_00[] = {0xE5,0x0E,0x16,0x23,0x2E,0x0E,0x07,0x1C,0x0A,0x08,0x07,0x09,0x19,0x05,0x0C,0x11,0x0D,0x0B,0x0B};
static char cmi_shift_E6_00[] = {0x00,0x00};
static char cmi_cmd_E6_00[] = {0xE6,0x0E,0x16,0x23,0x2E,0x0E,0x07,0x1C,0x0A,0x08,0x07,0x09,0x19,0x05,0x0C,0x11,0x0D,0x0B,0x0B};

static char cmi_shift_2A_4P[]={0x00, 0x00};
static char cmi_cmd_2A_4P[]={0x2A, 0x00, 0x00, 0x02, 0xcf};
static char cmi_shift_2B_4P[]={0x00, 0x00};
static char cmi_cmd_2B_4P[]={0x2b, 0x00, 0x00, 0x04, 0xff};
static char cmi_shift_C1_80[] = {0x00, 0x80};
static char cmi_cmd_C1_80[] = {0xc1, 0x85};
static char cmi_shift_C5_D0[] = {0x00, 0xD0};
static char cmi_cmd_C5_D0[] = {0xc5,0x38};

static struct dsi_cmd_desc OTM1281_CMI_VIDEO_INIT_PARA[] = {
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_FF),cmi_cmd_FF},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_FF_80),cmi_shift_FF_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_FF_80),cmi_cmd_FF_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_B3_A0),cmi_shift_B3_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_B3_A0),cmi_cmd_B3_A0},

		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_truly_shift_b0),cmi_cmd_truly_shift_b0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_truly_cmd_b0),cmi_cmd_truly_cmd_b0},
		
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_80),cmi_shift_CB_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_80),cmi_cmd_CB_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_90),cmi_shift_CB_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_90),cmi_cmd_CB_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_A0),cmi_shift_CB_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_A0),cmi_cmd_CB_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_B0),cmi_shift_CB_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_B0),cmi_cmd_CB_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_C0),cmi_shift_CB_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_C0),cmi_cmd_CB_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_D0),cmi_shift_CB_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_D0),cmi_cmd_CB_D0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_E0),cmi_shift_CB_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_E0),cmi_cmd_CB_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_F0),cmi_shift_CB_F0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_F0),cmi_cmd_CB_F0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_80),cmi_shift_CC_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_80),cmi_cmd_CC_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_A0),cmi_shift_CC_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_A0),cmi_cmd_CC_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_C0),cmi_shift_CC_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_C0),cmi_cmd_CC_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_E0),cmi_shift_CC_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_E0),cmi_cmd_CC_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_90),cmi_shift_C1_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_90),cmi_cmd_C1_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_80),cmi_shift_C0_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_80),cmi_cmd_C0_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_90),cmi_shift_C0_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_90),cmi_cmd_C0_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_A0),cmi_shift_C0_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_A0),cmi_cmd_C0_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_80),cmi_shift_C2_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_80),cmi_cmd_C2_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_90),cmi_shift_C2_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_90),cmi_cmd_C2_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_A0),cmi_shift_C2_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_A0),cmi_cmd_C2_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_B0),cmi_shift_C2_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_B0),cmi_cmd_C2_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_C0),cmi_shift_C2_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_C0),cmi_cmd_C2_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_E0),cmi_shift_C2_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_E0),cmi_cmd_C2_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_B3),cmi_shift_C2_B3},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_B3),cmi_cmd_C2_B3},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_A2),cmi_shift_C1_A2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_A2),cmi_cmd_C1_A2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_B4),cmi_shift_C0_B4},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_B4),cmi_cmd_C0_B4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_80),cmi_shift_C5_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_80),cmi_cmd_C5_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_90),cmi_shift_C5_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_90),cmi_cmd_C5_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D8_00),cmi_shift_D8_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_D8_00),cmi_cmd_D8_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D9_00),cmi_shift_D9_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_D9_00),cmi_cmd_D9_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_C0),cmi_shift_C0_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_C0),cmi_cmd_C0_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_D0),cmi_shift_C0_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_D0),cmi_cmd_C0_D0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B6),cmi_shift_F5_B6},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B6),cmi_cmd_F5_B6},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_B3_B0),cmi_shift_B3_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_B3_B0),cmi_cmd_B3_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B0),cmi_shift_F5_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B0),cmi_cmd_F5_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B8),cmi_shift_F5_B8},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B8),cmi_cmd_F5_B8},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_94),cmi_shift_F5_94},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_94),cmi_cmd_F5_94},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_A2),cmi_shift_F5_A2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_A2),cmi_cmd_F5_A2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_90),cmi_shift_F5_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_90),cmi_cmd_F5_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_A0),cmi_shift_F5_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_A0),cmi_cmd_F5_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_36),cmi_cmd_36},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D6_80),cmi_shift_D6_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_D6_80),cmi_cmd_D6_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E1_00),cmi_shift_E1_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E1_00),cmi_cmd_E1_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E2_00),cmi_shift_E2_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E2_00),cmi_cmd_E2_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E3_00),cmi_shift_E3_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E3_00),cmi_cmd_E3_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E4_00),cmi_shift_E4_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E4_00),cmi_cmd_E4_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E5_00),cmi_shift_E5_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E5_00),cmi_cmd_E5_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E6_00),cmi_shift_E6_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E6_00),cmi_cmd_E6_00},
		
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_2A_4P),cmi_shift_2A_4P},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_2A_4P), cmi_cmd_2A_4P},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_2B_4P),cmi_shift_2B_4P},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_2B_4P), cmi_cmd_2B_4P},

		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_80),cmi_shift_C1_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_80),cmi_cmd_C1_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_D0),cmi_shift_C5_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_D0),cmi_cmd_C5_D0},
#ifdef COLOR_ENHANCE
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_83),truly_shift_83},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F4),truly_cmd_F4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D4),truly_cmd_D4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D5),truly_cmd_D5},

#endif	
		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep),exit_sleep},
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on},
};

static struct dsi_cmd_desc OTM1281_CMI_CMD_INIT_PARA[] = {
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_FF),cmi_cmd_FF},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_FF_80),cmi_shift_FF_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_FF_80),cmi_cmd_FF_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_B3_A0),cmi_shift_B3_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_B3_A0),cmi_cmd_B3_A0},

		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_truly_shift_b0),cmi_cmd_truly_shift_b0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_truly_cmd_b0),cmi_cmd_truly_cmd_b0},
		
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_80),cmi_shift_CB_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_80),cmi_cmd_CB_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_90),cmi_shift_CB_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_90),cmi_cmd_CB_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_A0),cmi_shift_CB_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_A0),cmi_cmd_CB_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_B0),cmi_shift_CB_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_B0),cmi_cmd_CB_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_C0),cmi_shift_CB_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_C0),cmi_cmd_CB_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_D0),cmi_shift_CB_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_D0),cmi_cmd_CB_D0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_E0),cmi_shift_CB_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_E0),cmi_cmd_CB_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CB_F0),cmi_shift_CB_F0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CB_F0),cmi_cmd_CB_F0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_80),cmi_shift_CC_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_80),cmi_cmd_CC_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_A0),cmi_shift_CC_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_A0),cmi_cmd_CC_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_C0),cmi_shift_CC_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_C0),cmi_cmd_CC_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_CC_E0),cmi_shift_CC_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_CC_E0),cmi_cmd_CC_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_90),cmi_shift_C1_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_90),cmi_cmd_C1_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_80),cmi_shift_C0_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_80),cmi_cmd_C0_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_90),cmi_shift_C0_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_90),cmi_cmd_C0_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_A0),cmi_shift_C0_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_A0),cmi_cmd_C0_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_80),cmi_shift_C2_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_80),cmi_cmd_C2_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_90),cmi_shift_C2_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_90),cmi_cmd_C2_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_A0),cmi_shift_C2_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_A0),cmi_cmd_C2_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_B0),cmi_shift_C2_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_B0),cmi_cmd_C2_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_C0),cmi_shift_C2_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_C0),cmi_cmd_C2_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_E0),cmi_shift_C2_E0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_E0),cmi_cmd_C2_E0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C2_B3),cmi_shift_C2_B3},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C2_B3),cmi_cmd_C2_B3},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_A2),cmi_shift_C1_A2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_A2),cmi_cmd_C1_A2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_B4),cmi_shift_C0_B4},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_B4),cmi_cmd_C0_B4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_80),cmi_shift_C5_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_80),cmi_cmd_C5_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_90),cmi_shift_C5_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_90),cmi_cmd_C5_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D8_00),cmi_shift_D8_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_D8_00),cmi_cmd_D8_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D9_00),cmi_shift_D9_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_D9_00),cmi_cmd_D9_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_C0),cmi_shift_C0_C0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_C0),cmi_cmd_C0_C0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C0_D0),cmi_shift_C0_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C0_D0),cmi_cmd_C0_D0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B6),cmi_shift_F5_B6},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B6),cmi_cmd_F5_B6},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_B3_B0),cmi_shift_B3_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_B3_B0),cmi_cmd_B3_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B0),cmi_shift_F5_B0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B0),cmi_cmd_F5_B0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_B8),cmi_shift_F5_B8},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_B8),cmi_cmd_F5_B8},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_94),cmi_shift_F5_94},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_94),cmi_cmd_F5_94},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_A2),cmi_shift_F5_A2},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_A2),cmi_cmd_F5_A2},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_90),cmi_shift_F5_90},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_90),cmi_cmd_F5_90},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_F5_A0),cmi_shift_F5_A0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_F5_A0),cmi_cmd_F5_A0},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_36),cmi_cmd_36},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_35),cmi_cmd_35},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_D6_80),cmi_shift_D6_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_cmd_D6_80),cmi_cmd_D6_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E1_00),cmi_shift_E1_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E1_00),cmi_cmd_E1_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E2_00),cmi_shift_E2_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E2_00),cmi_cmd_E2_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E3_00),cmi_shift_E3_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E3_00),cmi_cmd_E3_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E4_00),cmi_shift_E4_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E4_00),cmi_cmd_E4_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E5_00),cmi_shift_E5_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E5_00),cmi_cmd_E5_00},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_E6_00),cmi_shift_E6_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_E6_00),cmi_cmd_E6_00},
		
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_2A_4P),cmi_shift_2A_4P},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_2A_4P), cmi_cmd_2A_4P},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_2B_4P),cmi_shift_2B_4P},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_2B_4P), cmi_cmd_2B_4P},
/*
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C1_80),cmi_shift_C1_80},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C1_80),cmi_cmd_C1_80},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(cmi_shift_C5_D0),cmi_shift_C5_D0},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(cmi_cmd_C5_D0),cmi_cmd_C5_D0},
*/
#ifdef COLOR_ENHANCE
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_83),truly_shift_83},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_F4),truly_cmd_F4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D4),truly_cmd_D4},
		{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(truly_shift_00),truly_shift_00},
		{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(truly_cmd_D5),truly_cmd_D5},

#endif	
		{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep),exit_sleep},
		{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_on), display_on},
};
uint32_t mipi_get_icpanleid(struct msm_fb_data_type *mfd )
{
	uint32_t panleid = 0;
	lcd_panle_reset();
	panleid =  mipi_read_commic_reg(mfd,&OTM_CMI_panelid_rd_cmd, 2,1);
	printk("debug read panleid is %x\n",panleid & 0xffffffff);
	switch(panleid & 0xffff)
	{
		case 0x8012:
			printk("lizhiye, mipi_get_icpanleid, LCD_PANEL_4P3_OTM1280_OT_CMI = %d\n", LCD_PANEL_4P3_OTM1280_OT_CMI);
			return LCD_PANEL_4P3_OTM1280_OT_CMI;	//cmi
		case 0x8112:
			printk("lizhiye, mipi_get_icpanleid, LCD_PANEL_4P3_OTM1281_OT_TRULY = %d\n", LCD_PANEL_4P3_OTM1281_OT_TRULY);
			return LCD_PANEL_4P3_OTM1281_OT_TRULY;	//ovi, truly
		default:
			break;
	}
	return 0;
}
uint8 mipi_read_otm_id(struct msm_fb_data_type *mfd)
{
	uint32_t reg1 = 0,reg2 = 0;
	reg1 =  mipi_read_commic_reg(mfd,&get_otm1281_0x0a_value_cmd, 1,0);
	reg2 =  mipi_read_commic_reg(mfd,&get_otm1281_0x0b_value_cmd, 1,0);
	//printk("debug read reg[0xa]= [%x]   reg[0xb] = [%x]\n",reg1&0xff,reg2&0xff);
	if(((reg1&0xff)!=0x9c)&&((reg2&0xff)!=0xd0)){
		pr_err("%s  failed  reg[0xa]= [%x]   reg[0xb] = [%x]\n", __func__,reg1&0xff,reg2&0xff);
		return -1;
	}else
	 	return 0;
	
}
static uint32 otm1281_read_panel_reg(struct msm_fb_data_type *mfd, char reg,char len) 
{ 
        struct dsi_buf *rp, *tp; 
        struct dsi_cmd_desc *cmd = NULL; 
        unsigned int  *lp; 

        tp = &mipi_tx_buf; 
        rp = &mipi_rx_buf;
	 memset(rp->data,0,rp->len);
        mipi_dsi_buf_init(tp); 
        mipi_dsi_buf_init(rp); 

        switch(reg) 
        { 
                case 0x0a: 
                        cmd = &get_otm1281_0x0a_value_cmd; 
                break; 
                default: 
                        cmd = &get_otm1281_0x0b_value_cmd; 
		   break; 
        } 

       mipi_dsi_cmds_rx(mfd, tp, rp, cmd, len); 
       lp = (unsigned int  *)rp->data; 
        return *lp; 
}

uint8 otm1281_esd_video_func(struct msm_fb_data_type *mfd) 
{
	unsigned int dsi_ctrl, ctrl;
	int video_mode,reg1,reg2;
	dsi_ctrl = readl(MIPI_DSI_BASE + 0x0000);
	video_mode = dsi_ctrl & 0x02; /* VIDEO_MODE_EN */
	if (video_mode) {
		ctrl = dsi_ctrl | 0x04; /* CMD_MODE_EN */
		writel(ctrl,MIPI_DSI_BASE + 0x0000);
		}
	mipi_set_tx_power_mode(0);
	reg1 =  otm1281_read_panel_reg(mfd,0x0a,1);
	reg2 =  otm1281_read_panel_reg(mfd,0x0b,1);
	mipi_set_tx_power_mode(1);
	if (video_mode)
		writel(dsi_ctrl,MIPI_DSI_BASE + 0x0000); /* restore */
	if(((reg1&0xff)!=0x9c)&&((reg2&0xff)!=0xd0)){
		pr_err("%s esd failed reg[0xa]= [%x]   reg[0xb] = [%x]\n", __func__,reg1&0xff,reg2&0xff);
		return -1;
	}else
	 	return 0;


}


 void lcd_esd_work_func(struct work_struct *work)
{	
	int ret = 0;
	DEV_DBG("lcd_esd_work_func\n");
	if((local_mfd!=NULL)&&!timer_deleted){
			ret = local_mfd->panel_info.is_sw_resolve_esd_func(local_mfd);
	}else
		return ;
		
		
	if(ret){
		if(!Lcd_display_mode)
			lcd_reset_esd(local_mfd);
		mipi_set_backlight(local_mfd);
	}

	if(!timer_deleted)
		mod_timer(&lcd_esd_timer,  jiffies+HZ*2);

}
  void lcd_esd_state_timer(unsigned long data)
{
	queue_work(lcd_esd_queue, &lcd_esd_work);		
		
}

void	mipi_tx_init_to_lcd(struct msm_fb_data_type *mfd)
{

	if(panel_init_para)
		mipi_dsi_cmds_tx(mfd, &mipi_tx_buf, panel_init_para, panel_init_para_len);
	else
		printk("LCD: not found the lcd id, error\n");
	mipi_read_otm_id(mfd);
}
void	lcd_init_prepare(u32 LcdPanleID)
{


	
	switch(LcdPanleID)
	{				
		case LCD_PANEL_4P3_OTM1280_OT_CMI:	//310
			printk("LCD: LCD_PANEL_4P3_OTM1280_OT_CMI\n");
			if(Lcd_display_mode){
				panel_init_para = OTM1281_CMI_CMD_INIT_PARA;
				panel_init_para_len = ARRAY_SIZE(OTM1281_CMI_CMD_INIT_PARA);
			}else {
				panel_init_para = OTM1281_CMI_VIDEO_INIT_PARA;
				panel_init_para_len = ARRAY_SIZE(OTM1281_CMI_VIDEO_INIT_PARA);
				lcdinit_backlight_delay = 60;
			}
			
			break;
		case LCD_PANEL_4P3_OTM1281_OT_TRULY:	//311
			printk("LCD: LCD_PANEL_4P3_OTM1281_OT_TRULY\n");
			if(Lcd_display_mode){
				panel_init_para = OTM1281_AUO_CMD_INIT_PARA;
				panel_init_para_len = ARRAY_SIZE(OTM1281_AUO_CMD_INIT_PARA);
			}else {
				panel_init_para = OTM1281_AUO_VIDEO_INIT_PARA;
				panel_init_para_len = ARRAY_SIZE(OTM1281_AUO_VIDEO_INIT_PARA);
				lcdinit_backlight_delay = 60;
			}
			lcdinit_backlight_delay = 0;

			break;		
		default:
			printk("LCD: not found the lcd id, error\n");
			break;
	}
}


