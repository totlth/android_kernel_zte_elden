/***********************************************************************************/
/*  Copyright (c) 2010, Silicon Image, Inc.  All rights reserved.             */
/*  No part of this work may be reproduced, modified, distributed, transmitted,    */
/*  transcribed, or translated into any language or computer format, in any form   */
/*  or by any means without written permission of: Silicon Image, Inc.,            */
/*  1060 East Arques Avenue, Sunnyvale, California 94085                           */
/***********************************************************************************/
#ifndef __MHL_SII8334_H__
#define __MHL_SII8334_H__
#include <linux/types.h>
#include <linux/kernel.h>




typedef _Bool bool_t;

#define LOW                     0
#define HIGH                    1

//------------------------------------------------------------------------------
// Configuration defines used by hal_config.h
//------------------------------------------------------------------------------

#define BIT0                    0x01
#define BIT1                    0x02
#define BIT2                    0x04
#define BIT3                    0x08
#define BIT4                    0x10
#define BIT5                    0x20
#define BIT6                    0x40
#define BIT7                    0x80

#define MSG_ALWAYS              0x00
#define MSG_STAT                0x01
#define MSG_DBG                 0x02

// see include/i2c_slave_addrs.h

#define SET_BITS    0xFF
#define CLEAR_BITS  0x00


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Debug Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*\
| | Debug Print Macro
| |
| | Note: TX_DEBUG_PRINT Requires double parenthesis
| | Example:  TX_DEBUG_PRINT(("hello, world!\n"));
\*/
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Debug Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define DISABLE 0x00
#define ENABLE  0xFF

// Compile debug prints inline or not
#define CONF__TX_DEBUG_PRINT   (DISABLE)

#if (CONF__TX_DEBUG_PRINT == ENABLE)
    #define TX_DEBUG_PRINT(x)	printk x
#else
    #define TX_DEBUG_PRINT(x)
#endif

#define CONF__CBUS_DEBUG_PRINT   (DISABLE)

#if (CONF__CBUS_DEBUG_PRINT == ENABLE)
    #define CBUS_DEBUG_PRINT(x)	printk x
#else
    #define CBUS_DEBUG_PRINT(x)
#endif


#define CONF__TX_I2C_DEBUG_PRINT   (DISABLE)
#if (CONF__TX_I2C_DEBUG_PRINT == ENABLE)
    #define TX_I2C_DEBUG_PRINT(x)	printk x
#else
    #define TX_I2C_DEBUG_PRINT(x)
#endif


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define RCP_ENABLE 	1

//====================================================
// VBUS power check for supply or charge
//====================================================
#define 	VBUS_POWER_CHK		(ENABLE) //(DISABLE)

void		HalTimerWait ( uint16_t m_sec );

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// System Board Definitions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define SB_NONE				(0)
#define SB_EPV5_MARK_II		(1)
#define SB_STARTER_KIT_X01	(2)

#define SYSTEM_BOARD		(SB_STARTER_KIT_X01)

#if (SYSTEM_BOARD == SB_EPV5_MARK_II)
#define SiI_TARGET_STRING       "SiI8334 EPV5 MARK II"
#elif (SYSTEM_BOARD == SB_STARTER_KIT_X01)
#define SiI_TARGET_STRING       "SiI8334 Starter Kit X01"
#else
#error "Unknown SYSTEM_BOARD definition in defs.h"
#endif

// ~~~~~~~~~~~

#define T_MONITORING_PERIOD		100
//
// This is the time in milliseconds we poll what we poll.
//
#define MONITORING_PERIOD		50

#define SiI_DEVICE_ID			0x8334

#define TX_HW_RESET_PERIOD		10

#define GLOBAL_BYTE_BUF_BLOCK_SIZE  131


extern unsigned char g_CommData[];


#if (VBUS_POWER_CHK == ENABLE)
///////////////////////////////////////////////////////////////////////////////
//
// AppVbusControl
//
// This function or macro is invoked from MhlTx driver to ask application to
// control the VBUS power. If powerOn is sent as non-zero, one should assume
// peer does not need power so quickly remove VBUS power.
//
// if value of "powerOn" is 0, then application must turn the VBUS power on
// within 50ms of this call to meet MHL specs timing.
//
// Application module must provide this function.
//
void	AppVbusControl( bool_t powerOn );
#endif

/*
	I2C client for sii8334
*/
extern struct i2c_client *sii8334_PAGE_TPI;
extern struct i2c_client *sii8334_PAGE_TX_L0;
extern struct i2c_client *sii8334_PAGE_TX_L1;
extern struct i2c_client *sii8334_PAGE_TX_2;
extern struct i2c_client *sii8334_PAGE_TX_3;
extern struct i2c_client *sii8334_PAGE_CBUS;


#define MHL_DEVICE_CATEGORY             (MHL_DEV_CAT_SOURCE)

enum
{
    	MHL_RCP_CMD_SELECT          = 0x00,
    	MHL_RCP_CMD_UP              = 0x01,
    	MHL_RCP_CMD_DOWN            = 0x02,
    	MHL_RCP_CMD_LEFT            = 0x03,
    	MHL_RCP_CMD_RIGHT           = 0x04,
    	MHL_RCP_CMD_RIGHT_UP        = 0x05,
    	MHL_RCP_CMD_RIGHT_DOWN      = 0x06,
    	MHL_RCP_CMD_LEFT_UP         = 0x07,
    	MHL_RCP_CMD_LEFT_DOWN       = 0x08,
    	MHL_RCP_CMD_ROOT_MENU       = 0x09,
    	MHL_RCP_CMD_SETUP_MENU      = 0x0A,
    	MHL_RCP_CMD_CONTENTS_MENU   = 0x0B,
    	MHL_RCP_CMD_FAVORITE_MENU   = 0x0C,
    	MHL_RCP_CMD_EXIT            = 0x0D,
	
	//0x0E - 0x1F are reserved

    	MHL_RCP_CMD_NUM_0           = 0x20,
    	MHL_RCP_CMD_NUM_1           = 0x21,
    	MHL_RCP_CMD_NUM_2           = 0x22,
    	MHL_RCP_CMD_NUM_3           = 0x23,
    	MHL_RCP_CMD_NUM_4           = 0x24,
    	MHL_RCP_CMD_NUM_5           = 0x25,
    	MHL_RCP_CMD_NUM_6           = 0x26,
    	MHL_RCP_CMD_NUM_7           = 0x27,
    	MHL_RCP_CMD_NUM_8           = 0x28,
    	MHL_RCP_CMD_NUM_9           = 0x29,

    	MHL_RCP_CMD_DOT             = 0x2A,
    	MHL_RCP_CMD_ENTER           = 0x2B,
    	MHL_RCP_CMD_CLEAR           = 0x2C,

	//0x2D - 0x2F are reserved

    	MHL_RCP_CMD_CH_UP           = 0x30,
    	MHL_RCP_CMD_CH_DOWN         = 0x31,
    	MHL_RCP_CMD_PRE_CH          = 0x32,
    	MHL_RCP_CMD_SOUND_SELECT    = 0x33,
    	MHL_RCP_CMD_INPUT_SELECT    = 0x34,
    	MHL_RCP_CMD_SHOW_INFO       = 0x35,
    	MHL_RCP_CMD_HELP            = 0x36,
    	MHL_RCP_CMD_PAGE_UP         = 0x37,
    	MHL_RCP_CMD_PAGE_DOWN       = 0x38,

	//0x39 - 0x40 are reserved

    	MHL_RCP_CMD_VOL_UP	        = 0x41,
    	MHL_RCP_CMD_VOL_DOWN        = 0x42,
    	MHL_RCP_CMD_MUTE            = 0x43,
    	MHL_RCP_CMD_PLAY            = 0x44,
    	MHL_RCP_CMD_STOP            = 0x45,
    	MHL_RCP_CMD_PAUSE           = 0x46,
    	MHL_RCP_CMD_RECORD          = 0x47,
    	MHL_RCP_CMD_REWIND          = 0x48,
    	MHL_RCP_CMD_FAST_FWD        = 0x49,
    	MHL_RCP_CMD_EJECT           = 0x4A,
    	MHL_RCP_CMD_FWD             = 0x4B,
    	MHL_RCP_CMD_BKWD            = 0x4C,

	//0x4D - 0x4F are reserved

    	MHL_RCP_CMD_ANGLE            = 0x50,
    	MHL_RCP_CMD_SUBPICTURE       = 0x51,

	//0x52 - 0x5F are reserved

    	MHL_RCP_CMD_PLAY_FUNC       = 0x60,
    	MHL_RCP_CMD_PAUSE_PLAY_FUNC = 0x61,
    	MHL_RCP_CMD_RECORD_FUNC     = 0x62,
    	MHL_RCP_CMD_PAUSE_REC_FUNC  = 0x63,
    	MHL_RCP_CMD_STOP_FUNC       = 0x64,
    	MHL_RCP_CMD_MUTE_FUNC       = 0x65,
    	MHL_RCP_CMD_UN_MUTE_FUNC    = 0x66,
    	MHL_RCP_CMD_TUNE_FUNC       = 0x67,
    	MHL_RCP_CMD_MEDIA_FUNC      = 0x68,

	//0x69 - 0x70 are reserved

    	MHL_RCP_CMD_F1              = 0x71,
    	MHL_RCP_CMD_F2              = 0x72,
    	MHL_RCP_CMD_F3              = 0x73,
    	MHL_RCP_CMD_F4              = 0x74,
    	MHL_RCP_CMD_F5              = 0x75,

	//0x76 - 0x7D are reserved

    	MHL_RCP_CMD_VS              = 0x7E,
    	MHL_RCP_CMD_RSVD            = 0x7F,
};

typedef enum
{
    HAL_RET_SUCCESS,			/**< The operation was successfully completed */
    HAL_RET_FAILURE,			/**< Generic error */
    HAL_RET_PARAMETER_ERROR,	/**< Invalid parameter passed to a HAL function */
    HAL_RET_NO_DEVICE,			/**< Indicates that the specified hardware device was not found */
    HAL_RET_DEVICE_NOT_OPEN,	/**< The specified device is not open for use */
    HAL_RET_NOT_INITIALIZED,	/**< HAL abstraction layer module has not been initialized*/
    HAL_RET_OUT_OF_RESOURCES,	/**< Needed resources (memory/hardware) were not available. */
    HAL_RET_TIMEOUT,			/**< The requested operation timed out */
    HAL_RET_ALREADY_INITIALIZED /**< HalInit called more than once */
} halReturn_t;
halReturn_t I2cAccessCheck(void);

#endif  // __MHL_SII8334_H__

