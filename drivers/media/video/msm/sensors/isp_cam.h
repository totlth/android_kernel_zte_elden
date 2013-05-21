/*
 * drivers/media/video/msm/isp_800.h
 *
 * Refer to drivers/media/video/msm/mt9d112.h
 * For MT9T111: 3.1Mp, 1/4-Inch System-On-A-Chip (SOC) CMOS Digital Image Sensor
 * For MT9T112: 3.1Mp, 1/4-Inch System-On-A-Chip (SOC) CMOS Digital Image Sensor
 *
 * Copyright (C) 2009-2010 ZTE Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Created by jia.jia@zte.com.cn
 */

#ifndef ISP_cam_H
#define ISP_cam_H

#include <mach/board.h>
#include <mach/camera.h>
#include <linux/file.h>
#include <linux/fs.h>


/** The length of the header of the I2C write memory command (8 bit access) */
#define M7MO_WRITEMEMORY_START		8 

// M7MO Host Interrupt Factor
#define	M7MO_HOSTINT_MON			(0x0001)	/**< MonitorMode change */
#define	M7MO_HOSTINT_AF				(0x0002)	/**< AF operation(After focusing) */
#define	M7MO_HOSTINT_ZOOM			(0x0004)	/**< Zoom operation((when reach edge) */
#define	M7MO_HOSTINT_CAPTURE		(0x0008)	/**< Capture operation */
#define	M7MO_HOSTINT_FRAMESYNC		(0x0010)	/**< Frame sync(frame output in dual capture) */
#define	M7MO_HOSTINT_FD				(0x0020)	/**< Face detection (When detecting faces) */
#define	M7MO_HOSTINT_LENSINIT		(0x0040)	/**< Lens Initialize */
#define	M7MO_HOSTINT_SOUND			(0x0080)	/**< Shutter Sound and AF complete */
#define M7MO_HOSTINT_ALL			(0x00FF)	/**< Superset of all factors */

// interrupt waiting mode
#define M7MO_WAIT_AND				(0x0000)	/**< Wait until all interrupt factors received */
#define M7MO_WAIT_OR				(0x0001)	/**< Wait until any interrupt factor received */
#define M7MO_WAIT_PEEK				(0x0002) 	/**< Check if the specified interrupt are received and return immediately */


/**
 * Option of YUV output clock frequency
 * @note Value of this enum is important
 */ 
typedef enum {
										// if MCLK = 27MHz 
	E_M7MO_YUVClockOption_Slow = 0,		/**< 20.25MHz */
	E_M7MO_YUVClockOption_Normal = 1,  	/**< 27MHz */
	E_M7MO_YUVClockOption_32_40 = 2,	/**< 32.4MHz  */
	E_M7MO_YUVClockOption_40_50 = 3,	/**< 40.5MHz  */
	E_M7MO_YUVClockOption_46_28 = 4,	/**< 46.28MHz */
	E_M7MO_YUVClockOption_54_00 = 5,	/**< 54MHz    */
	E_M7MO_YUVClockOption_64_80 = 6,	/**< 64.8MHz  */
	E_M7MO_YUVClockOption_Fast = 0x19,  /**< ~72MHz */
} E_M7MO_YUVClockOption;


/**
 * AF mode 
 * @note Value of this enum is important
 */
typedef enum {
	E_M7MO_AF_MODE_RANGE = 0x00, 	// use AF_RANGE
	E_M7MO_AF_MODE_NORMAL_AF, 		// normal AF
	E_M7MO_AF_MODE_NORMAL_AF_FAST, 	// normal AF (fast)
	E_M7MO_AF_MODE_MACRO_AF, 		// macro AF
	E_M7MO_AF_MODE_MACRO_AF_FAST, 	// macro AF (fast)
	E_M7MO_AF_MODE_FULL_AF, 		// Full range AF
	E_M7MO_AF_MODE_CONT_AF, 		// Continuous AF
} E_M7MO_AF_MODE;

/**
 * AF window
 * @note Value of this enum is important
 */
typedef enum {
	E_M7MO_AF_WINDOW_CENTER_SMALL = 0x00, 	// centre small
	E_M7MO_AF_WINDOW_CENTER_LARGE, 			// centre large
	E_M7MO_AF_WINDOW_MUILTI, 				// 5-points multi AF
	E_M7MO_AF_WINDOW_FD, 					// Face detection AF
	E_M7MO_AF_WINDOW_TOUCH,				 	// Touch AF
} E_M7MO_AF_WINDOW;

/**
 * AF scan mode
 * @note Value of this enum is important
 */
typedef enum {
	E_M7MO_AF_SCAN_MODE_FULL = 0x00, 		// full scan
	E_M7MO_AF_SCAN_MODE_RESERVED_1, 		// reserved
	E_M7MO_AF_SCAN_MODE_RESERVED_2, 		// reserved
	E_M7MO_AF_SCAN_MODE_FAST, 				// fast scan
	E_M7MO_AF_SCAN_MODE_CONTINOUS, 			// continuous scan
	E_M7MO_AF_SCAN_MODE_CONTINOUS_FINE, 	// continuous scan with fine movement
} E_M7MO_AF_SCAN_MODE;

/**
 * AF range mode
 * @note Value of this enum is important
 */
typedef enum {
	E_M7MO_AF_RANGE_MODE_NORMAL = 0x00, 	// normal range
	E_M7MO_AF_RANGE_MODE_MACRO, 			// macro range
	E_M7MO_AF_RANGE_MODE_FULL,		 		// full range
} E_M7MO_AF_RANGE_MODE;




/** 
 * Switch interrupt process method.
 * If disable, this library polls the interrupt status of M7MO periodically.
 * If enable, this library listens the interrupt pin of M7MO and check the 
 * interrupt status only when there is interrupt.
 */
//#define M7MO_USE_INT

/**
 * Enable source codes dedicated for Fujitsu Host.
 * @note Remark this comment when releasing to client 
 */
//#define FUJITSU_HOST

#ifdef FUJITSU_HOST
/**
 * Debug message printing functions
 */
#define M7MO_DEBUG_PRINT1(a1)			pr_err(a1)
#define M7MO_DEBUG_PRINT2(a1,a2)		pr_err(a1,a2)
#define M7MO_DEBUG_PRINT3(a1,a2,a3)		pr_err(a1,a2,a3)
#define M7MO_DEBUG_PRINT4(a1,a2,a3,a4) 	pr_err(a1,a2,a3,a4)
#else
// TODO fill in the custom debug message printing functions, as above
#define M7MO_DEBUG_PRINT1(a1)			
#define M7MO_DEBUG_PRINT2(a1,a2)			
#define M7MO_DEBUG_PRINT3(a1,a2,a3)		
#define M7MO_DEBUG_PRINT4(a1,a2,a3,a4)
#endif

#define M7MO_OK		0
#define M7MO_NG		1

#define M7MO_FALSE	0
#define M7MO_TRUE	1

#define	M7MO_PARA_MODE_ERR		(0)
#define	M7MO_PARA_MODE_PAR		(0x01)
#define	M7MO_PARA_MODE_MON		(0x02)
#define	M7MO_PARA_MODE_CAP		(0x03)

typedef enum {
	E_M7MO_Mode_Parameter,				/**< Parameter mode */
	E_M7MO_Mode_Monitor,				/**< Monitor mode */
	E_M7MO_Mode_Capture,				/**< Capture mode */
} E_M7MO_Mode;

#define	M7MO_HOST_SUM_MAXSIZE		(0x8000)
#define	M7MO_FW_M7MORAM_ADDRESS		(0x68000000)	// M7MO internal ram address
#define WRITE_BUF_ADDR				(0x10000000)
#define WRITE_BUF_SIZE				(0x001F8000)

typedef	struct	{
	unsigned long	start_addr;
	unsigned long	sector_size;
	int				sector_count;
}	T_M7MO_FLASH_SECTOR_INFO;

T_M7MO_FLASH_SECTOR_INFO	gM7MO_SectorInfo[] = {
					/*	base mask,  sector mask,  0 */
					{	0xFFE00000,	0x003FE000,	  0	},
					/*	start_addr, sector_size, sector_count */
	/* 64K Byte */	{	0x00000000,	0x00010000,	 31	},
	/*  8K Byte */	{	0x001F0000,	0x00002000,	  8	},
	/* (eot)    */	{	0xffffffff,	0xffffffff,	 -1	}
};

// TODO customize the settings of memory buffer for placing the loaded firmware
#define READ_BUF_ADDR	0x80FFFFF8 // the address of memory buffer for placing firmware
#define READ_BUF_SIZE 	0x00200000 // the size of the above buffer



/**
 *	@brief		Write one byte to specified category and byte
 *  @param[in]	cat			Indicate which category to be written
 *  @param[in]	byte		Indicate which byte of the category to be written
 *  @param[in]	data		value to be written
 *	@return		None
 *	@note		None
 *	@attention
*/
int M7MO_WriteOneByteCRAM(unsigned char cat,unsigned char byte,unsigned char data);


/**
 *	@brief		Read one byte from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		The byte required to be read
 *	@note		None
 *	@attention
*/
unsigned char M7MO_ReadOneByteCRAM(unsigned char cat,unsigned char byte);


/**
 *	@brief		Write two byte to specified category and byte
 *  @param[in]	cat			Indicate which category to be written
 *  @param[in]	byte		Indicate the starting byte of the category to be written
 *  @param[in]	data		value to be written
 *	@return		None
 *	@note		None
 *	@attention
*/
int M7MO_WriteOneHalfwordCRAM( unsigned char cat, unsigned char byte, unsigned short data );



/**
 *	@brief		Read two byte from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		The byte required to be read
 *	@note		None
 *	@attention
*/
unsigned short M7MO_ReadOneHalfwordCRAM( unsigned char cat, unsigned char byte );



/**
 *	@brief		Write four byte to specified category and byte
 *  @param[in]	cat			Indicate which category to be written
 *  @param[in]	byte		Indicate the starting byte of the category to be written
 *  @param[in]	data		value to be written
 *	@return		None
 *	@note		None
 *	@attention
*/
int M7MO_WriteOneWordCRAM( unsigned char cat, unsigned char byte, unsigned long data );



/**
 *	@brief		Read four byte from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		The byte required to be read
 *	@note		None
 *	@attention
*/
unsigned long M7MO_ReadOneWordCRAM( unsigned char cat, unsigned char byte );

/**
 *	@brief		Read bytes from specified category and byte
 *  @param[in]	cat			Indicate which category to be read
 *  @param[in]	byte		Indicate which byte of the category to be read
 *	@return		pointer to the received data
 *	@note		None
 *	@attention
*/
void M7MO_ReadBytesCRAM( unsigned char cat, unsigned char byte , unsigned char byteCnt, unsigned char* cmd_recdata);



/*
        file I/O in kernel module
        by flyduck 2001/03/21
*/

//#define __KERNEL__
//#define MODULE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>

#include <asm/processor.h>
#include <asm/uaccess.h>

#define EOF             (-1)
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

//
// Function Prototypes
//

struct file *klib_fopen(const char *filename, int flags, int mode);
void klib_fclose(struct file *filp);

int klib_fseek(struct file *filp, int offset, int whence);

int klib_fread(char *buf, int len, struct file *filp);
int klib_fgetc(struct file *filp);
char *klib_fgets(char *str, int size, struct file *filp);

int klib_fwrite(char *buf, int len, struct file *filp);
int klib_fputc(int ch, struct file *filp);
int klib_fputs(char *str, struct file *filp);
int klib_fprintf(struct file *filp, const char *fmt, ...);


#endif /* ISP_CAM_H */
