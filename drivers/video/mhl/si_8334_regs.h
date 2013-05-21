/*
 *****************************************************************************
 *
 * Copyright 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  TPI_Regs.h
 *
 * @brief Implementation of the Foo API.
 *
 *****************************************************************************
*/

/***********************************************************************************/
/*  Copyright (c) 2002-2009, Silicon Image, Inc.  All rights reserved.             */
/*  No part of this work may be reproduced, modified, distributed, transmitted,    */
/*  transcribed, or translated into any language or computer format, in any form   */
/*  or by any means without written permission of: Silicon Image, Inc.,            */
/*  1060 East Arques Avenue, Sunnyvale, California 94085                           */
/***********************************************************************************/

#define REG_SRST		(TX_PAGE_3 | 0x0000)		/* Was 0x0005 */

#define REG_DEV_IDL     (TX_PAGE_L0| 0x0002)
#define REG_DEV_IDH     (TX_PAGE_L0| 0x0003)
#define REG_DEV_REV     (TX_PAGE_L0| 0x0004)

#define REG_SYS_CTRL1   (TX_PAGE_L0| 0x0008)

#define REG_DCTL        (TX_PAGE_L0| 0x000D)

#define REG_INTR_STATE  (TX_PAGE_L0| 0x0070)
#define REG_INTR1       (TX_PAGE_L0| 0x0071)
#define REG_INTR1_MASK  (TX_PAGE_L0| 0x0075)
#define REG_INTR2       (TX_PAGE_L0| 0x0072)
#define     INTR2_PSTABLE 0x02
#define REG_TMDS_CCTRL  (TX_PAGE_L0| 0x0080)
#define REG_TMDS_CTRL4  (TX_PAGE_L0| 0x0085)
#define REG_USB_CHARGE_PUMP (TX_PAGE_L0 | 0x00F8)

#define REG_DPD         (TX_PAGE_L1| 0x003D)


#define REG_TMDS0_CCTRL2    (TX_PAGE_2 | 0x0000)

#define REG_DVI_CTRL3	(TX_PAGE_2 | 0x0005)

#define REG_TMDS_CLK_EN (TX_PAGE_2 | 0x0011)
#define REG_TMDS_CH_EN  (TX_PAGE_2 | 0x0012)
#define REG_TMDS_TERMCTRL1 (TX_PAGE_2 | 0x0013)

#define REG_PLL_CALREFSEL  (TX_PAGE_2 | 0x0017)
#define REG_PLL_VCOCAL  (TX_PAGE_2 | 0x001A)

#define REG_EQ_DATA0    (TX_PAGE_2 | 0x0022)
#define REG_EQ_DATA1    (TX_PAGE_2 | 0x0023)
#define REG_EQ_DATA2    (TX_PAGE_2 | 0x0024)
#define REG_EQ_DATA3    (TX_PAGE_2 | 0x0025)
#define REG_EQ_DATA4    (TX_PAGE_2 | 0x0026)
#define REG_EQ_DATA5    (TX_PAGE_2 | 0x0027)
#define REG_EQ_DATA6    (TX_PAGE_2 | 0x0028)
#define REG_EQ_DATA7    (TX_PAGE_2 | 0x0029)

#define REG_BW_I2C      (TX_PAGE_2 | 0x0031)

#define REG_EQ_PLL_CTRL1    (TX_PAGE_2 | 0x0045)

#define REG_MON_USE_COMP_EN (TX_PAGE_2 | 0x004B)
#define REG_ZONE_CTRL_SW_RST (TX_PAGE_2 | 0x004C)
#define REG_MODE_CONTROL    (TX_PAGE_2 | 0x004D)

#define REG_DISC_CTRL1	(TX_PAGE_3 | 0x0010)		/* Was 0x0090 */
#define REG_DISC_CTRL2	(TX_PAGE_3 | 0x0011)		/* Was 0x0091 */
#define REG_DISC_CTRL3	(TX_PAGE_3 | 0x0012)		/* Was 0x0092 */
#define REG_DISC_CTRL4	(TX_PAGE_3 | 0x0013)		/* Was 0x0093 */
#define REG_DISC_CTRL5	(TX_PAGE_3 | 0x0014)		/* Was 0x0094 */
#define REG_DISC_CTRL6	(TX_PAGE_3 | 0x0015)		/* Was 0x0095 */
#define REG_DISC_CTRL7	(TX_PAGE_3 | 0x0016)		/* Was 0x0096 */
#define REG_DISC_CTRL8	(TX_PAGE_3 | 0x0017)		/* Was 0x0097 */
#define REG_DISC_CTRL9	(TX_PAGE_3 | 0x0018)
#define REG_DISC_CTRL10	(TX_PAGE_3 | 0x0019)
#define REG_DISC_CTRL11	(TX_PAGE_3 | 0x001A)
#define REG_DISC_STAT	(TX_PAGE_3 | 0x001B)		/* Was 0x0098 */
#define REG_DISC_STAT2	(TX_PAGE_3 | 0x001C)		/* Was 0x0099 */

#define REG_INT_CTRL	(TX_PAGE_3 | 0x0020)		/* Was 0x0079 */
#define REG_INTR4		(TX_PAGE_3 | 0x0021)		/* Was 0x0074 */
#define     INTR4_SCDT_CHANGE 0x01
#define REG_INTR4_MASK	(TX_PAGE_3 | 0x0022)		/* Was 0x0078 */
#define REG_INTR5		(TX_PAGE_3 | 0x0023)
#define REG_INTR5_MASK	(TX_PAGE_3 | 0x0024)

#define REG_MHLTX_CTL1	(TX_PAGE_3 | 0x0030)		/* Was 0x00A0 */
#define REG_MHLTX_CTL2	(TX_PAGE_3 | 0x0031)		/* Was 0x00A1 */
#define REG_MHLTX_CTL3	(TX_PAGE_3 | 0x0032)		/* Was 0x00A2 */
#define REG_MHLTX_CTL4	(TX_PAGE_3 | 0x0033)		/* Was 0x00A3 */
#define REG_MHLTX_CTL5	(TX_PAGE_3 | 0x0034)		/* Was 0x00A4 */
#define REG_MHLTX_CTL6	(TX_PAGE_3 | 0x0035)		/* Was 0x00A5 */
#define REG_MHLTX_CTL7	(TX_PAGE_3 | 0x0036)		/* Was 0x00A6 */
#define REG_MHLTX_CTL8	(TX_PAGE_3 | 0x0037)

#define REG_TMDS_CSTAT	(TX_PAGE_3 | 0x0040)
#define     TMDS_CSTAT_SCDT 0x02

#define REG_MIPI_DSI_CTRL		(TX_PAGE_3 | 0x0092)
#define REG_MIPI_DSI_FORMAT		(TX_PAGE_3 | 0x0093)
#define REG_MIPI_DSI_PXL_FORMAT	(TX_PAGE_3 | 0x0094)

// ===================================================== //

#define TPI_SYSTEM_CONTROL_DATA_REG			(TX_PAGE_TPI | 0x001A)

#define LINK_INTEGRITY_MODE_MASK			(BIT6)
#define LINK_INTEGRITY_STATIC				(0x00)
#define LINK_INTEGRITY_DYNAMIC				(0x40)

#define TMDS_OUTPUT_CONTROL_MASK			(BIT4)
#define TMDS_OUTPUT_CONTROL_ACTIVE			(0x00)
#define TMDS_OUTPUT_CONTROL_POWER_DOWN		(0x10)

#define AV_MUTE_MASK						(BIT3)
#define AV_MUTE_NORMAL						(0x00)
#define AV_MUTE_MUTED						(0x08)

#define DDC_BUS_REQUEST_MASK				(BIT2)
#define DDC_BUS_REQUEST_NOT_USING			(0x00)
#define DDC_BUS_REQUEST_REQUESTED			(0x04)

#define DDC_BUS_GRANT_MASK					(BIT1)
#define DDC_BUS_GRANT_NOT_AVAILABLE			(0x00)
#define DDC_BUS_GRANT_GRANTED				(0x02)

#define OUTPUT_MODE_MASK					(BIT0)
#define OUTPUT_MODE_DVI						(0x00)
#define OUTPUT_MODE_HDMI					(0x01)

// ===================================================== //

#define TPI_DEVICE_POWER_STATE_CTRL_REG		(TX_PAGE_TPI | 0x001E)

#define CTRL_PIN_CONTROL_MASK				(BIT4)
#define CTRL_PIN_TRISTATE					(0x00)
#define CTRL_PIN_DRIVEN_TX_BRIDGE			(0x10)

#define TX_POWER_STATE_MASK					(BIT1 | BIT0)
#define TX_POWER_STATE_D0					(0x00)
#define TX_POWER_STATE_D2					(0x02)
#define TX_POWER_STATE_D3					(0x03)

/*\
| | HDCP Implementation
| |
| | HDCP link security logic is implemented in certain transmitters; unique
| |   keys are embedded in each chip as part of the solution. The security
| |   scheme is fully automatic and handled completely by the hardware.
\*/

/// HDCP Query Data Register ============================================== ///

#define TPI_HDCP_QUERY_DATA_REG				(TX_PAGE_TPI | 0x0029)

#define EXTENDED_LINK_PROTECTION_MASK		(BIT7)
#define EXTENDED_LINK_PROTECTION_NONE		(0x00)
#define EXTENDED_LINK_PROTECTION_SECURE		(0x80)

#define LOCAL_LINK_PROTECTION_MASK			(BIT6)
#define LOCAL_LINK_PROTECTION_NONE			(0x00)
#define LOCAL_LINK_PROTECTION_SECURE		(0x40)

#define LINK_STATUS_MASK					(BIT5 | BIT4)
#define LINK_STATUS_NORMAL					(0x00)
#define LINK_STATUS_LINK_LOST				(0x10)
#define LINK_STATUS_RENEGOTIATION_REQ		(0x20)
#define LINK_STATUS_LINK_SUSPENDED			(0x30)

#define HDCP_REPEATER_MASK					(BIT3)
#define HDCP_REPEATER_NO					(0x00)
#define HDCP_REPEATER_YES					(0x08)

#define CONNECTOR_TYPE_MASK					(BIT2 | BIT0)
#define CONNECTOR_TYPE_DVI					(0x00)
#define CONNECTOR_TYPE_RSVD					(0x01)
#define CONNECTOR_TYPE_HDMI					(0x04)
#define CONNECTOR_TYPE_FUTURE				(0x05)

#define PROTECTION_TYPE_MASK				(BIT1)
#define PROTECTION_TYPE_NONE				(0x00)
#define PROTECTION_TYPE_HDCP				(0x02)

/// HDCP Control Data Register ============================================ ///

#define TPI_HDCP_CONTROL_DATA_REG			(TX_PAGE_TPI | 0x002A)

#define PROTECTION_LEVEL_MASK				(BIT0)
#define PROTECTION_LEVEL_MIN				(0x00)
#define PROTECTION_LEVEL_MAX				(0x01)

#define KSV_FORWARD_MASK					(BIT4)
#define KSV_FORWARD_ENABLE					(0x10)
#define KSV_FORWARD_DISABLE					(0x00)

/// HDCP BKSV Registers =================================================== ///

#define TPI_BKSV_1_REG						(TX_PAGE_TPI | 0x002B)
#define TPI_BKSV_2_REG						(TX_PAGE_TPI | 0x002C)
#define TPI_BKSV_3_REG						(TX_PAGE_TPI | 0x002D)
#define TPI_BKSV_4_REG						(TX_PAGE_TPI | 0x002E)
#define TPI_BKSV_5_REG						(TX_PAGE_TPI | 0x002F)

/// HDCP Revision Data Register =========================================== ///

#define TPI_HDCP_REVISION_DATA_REG			(TX_PAGE_TPI | 0x0030)

#define HDCP_MAJOR_REVISION_MASK			(BIT7 | BIT6 | BIT5 | BIT4)
#define HDCP_MAJOR_REVISION_VALUE			(0x10)

#define HDCP_MINOR_REVISION_MASK			(BIT3 | BIT2 | BIT1 | BIT0)
#define HDCP_MINOR_REVISION_VALUE			(0x02)

/// HDCP KSV and V' Value Data Register =================================== ///

#define TPI_V_PRIME_SELECTOR_REG			(TX_PAGE_TPI | 0x0031)

/// V' Value Readback Registers =========================================== ///

#define TPI_V_PRIME_7_0_REG					(TX_PAGE_TPI | 0x0032)
#define TPI_V_PRIME_15_9_REG				(TX_PAGE_TPI | 0x0033)
#define TPI_V_PRIME_23_16_REG				(TX_PAGE_TPI | 0x0034)
#define TPI_V_PRIME_31_24_REG				(TX_PAGE_TPI | 0x0035)

/// HDCP AKSV Registers =================================================== ///

#define TPI_AKSV_1_REG						(TX_PAGE_TPI | 0x0036)
#define TPI_AKSV_2_REG						(TX_PAGE_TPI | 0x0037)
#define TPI_AKSV_3_REG						(TX_PAGE_TPI | 0x0038)
#define TPI_AKSV_4_REG						(TX_PAGE_TPI | 0x0039)
#define TPI_AKSV_5_REG						(TX_PAGE_TPI | 0x003A)

/// Interrupt Status Register ============================================= ///

#define TPI_INTERRUPT_STATUS_REG			(TX_PAGE_TPI | 0x003D)

#define HDCP_AUTH_STATUS_CHANGE_EVENT_MASK	(BIT7)
#define HDCP_AUTH_STATUS_CHANGE_EVENT_NO	(0x00)
#define HDCP_AUTH_STATUS_CHANGE_EVENT_YES	(0x80)

#define HDCP_VPRIME_VALUE_READY_EVENT_MASK	(BIT6)
#define HDCP_VPRIME_VALUE_READY_EVENT_NO	(0x00)
#define HDCP_VPRIME_VALUE_READY_EVENT_YES	(0x40)

#define HDCP_SECURITY_CHANGE_EVENT_MASK		(BIT5)
#define HDCP_SECURITY_CHANGE_EVENT_NO		(0x00)
#define HDCP_SECURITY_CHANGE_EVENT_YES		(0x20)

#define AUDIO_ERROR_EVENT_MASK				(BIT4)
#define AUDIO_ERROR_EVENT_NO				(0x00)
#define AUDIO_ERROR_EVENT_YES				(0x10)

#define CPI_EVENT_MASK						(BIT3)
#define CPI_EVENT_NO						(0x00)
#define CPI_EVENT_YES						(0x08)
#define RX_SENSE_MASK						(BIT3)		/* This bit is dual purpose depending on the value of 0x3C[3] */
#define RX_SENSE_NOT_ATTACHED				(0x00)
#define RX_SENSE_ATTACHED					(0x08)

#define HOT_PLUG_PIN_STATE_MASK				(BIT2)
#define HOT_PLUG_PIN_STATE_LOW				(0x00)
#define HOT_PLUG_PIN_STATE_HIGH				(0x04)

#define RECEIVER_SENSE_EVENT_MASK			(BIT1)
#define RECEIVER_SENSE_EVENT_NO				(0x00)
#define RECEIVER_SENSE_EVENT_YES			(0x02)

#define HOT_PLUG_EVENT_MASK					(BIT0)
#define HOT_PLUG_EVENT_NO					(0x00)
#define HOT_PLUG_EVENT_YES					(0x01)

///////////////////////////////////////////////////////////////////////////////
//
// CBUS register definitions
//

#define REG_CBUS_COMMON_CONFIG          (TX_PAGE_CBUS | 0x0007)
#define REG_CBUS_INTR_STATUS            (TX_PAGE_CBUS | 0x0008)
#define BIT_CBUS_CNX_CHG        BIT0
#define BIT_CBUS_CEC_ABRT       BIT1    /* Responder aborted DDC command at translation layer */
#define BIT_CBUS_DDC_ABRT       BIT2    /* Responder sent a VS_MSG packet (response data or command.) */
#define BIT_CBUS_MSC_MR_MSC_MSG BIT3    /* Responder sent ACK packet (not VS_MSG) */
#define BIT_CBUS_MSC_MT_DONE    BIT4    /* Command send aborted on TX side */
#define BIT_CBUS_MSC_MT_ABRT    BIT5    /* Responder aborted MSC command at translation layer */
#define BIT_CBUS_MSC_MR_ABRT    BIT6
#define BIT_CBUS_LINK_SOFT_ERR  BIT7

#define BIT_DDC_ABORT                   BIT_CBUS_DDC_ABRT
#define BIT_MSC_MSG_RCV                 BIT_CBUS_MSC_MR_MSC_MSG
#define BIT_MSC_XFR_DONE                BIT_CBUS_MSC_MT_DONE
#define BIT_MSC_XFR_ABORT               BIT_CBUS_MSC_MT_ABRT
#define BIT_MSC_ABORT                   BIT_CBUS_MSC_MR_ABRT

#define REG_CBUS_INTR_ENABLE            (TX_PAGE_CBUS | 0x0009)
#define BIT_CBUS_INTRP_CNX_CHG_EN        BIT0
#define BIT_CBUS_INTRP_CEC_ABRT_EN       BIT1
#define BIT_CBUS_INTRP_DDC_ABRT_EN       BIT2
#define BIT_CBUS_INTRP_MSC_MR_MSC_MSG_EN BIT3
#define BIT_CBUS_INTRP_MSC_MT_DONE_EN    BIT4
#define BIT_CBUS_INTRP_MSC_MT_ABRT_EN    BIT5
#define BIT_CBUS_INTRP_MSC_MR_ABRT_EN    BIT6
#define BIT_CBUS_INTRP_LINK_SOFT_ERR_EN  BIT7

#define REG_DDC_ABORT_REASON        	(TX_PAGE_CBUS | 0x000B)
#define REG_CBUS_BUS_STATUS             (TX_PAGE_CBUS | 0x000A)
#define BIT_BUS_CONNECTED                   0x01
#define BIT_LA_VAL_CHG                      0x02

#define REG_PRI_XFR_ABORT_REASON        (TX_PAGE_CBUS | 0x000D)

#define REG_CBUS_PRI_FWR_ABORT_REASON   (TX_PAGE_CBUS | 0x000E)
#define	BIT_CBUS_MSC_MT_MAX_FAIL                    BIT0
#define	BIT_CBUS_MSC_MT_PRORO_ERR                   BIT1
#define	BIT_CBUS_MSC_MT_TIMEOUT                     BIT2
#define	BIT_CBUS_MSC_UNDEF_CMD                      BIT3

#define BIT_CBUS_MSC_MT_CMD_LAT_FLUSHED_BY_HB_FAIL  BIT5
#define	BIT_CBUS_HPD                                BIT6
#define	BIT_CBUS_MSC_MT_ABRT_BY_PEER                BIT7

#define	CBUSABORT_BIT_REQ_MAXFAIL		BIT_CBUS_MSC_MT_MAX_FAIL
#define	CBUSABORT_BIT_PROTOCOL_ERROR	BIT_CBUS_MSC_MT_PRORO_ERR
#define	CBUSABORT_BIT_REQ_TIMEOUT	    BIT_CBUS_MSC_MT_TIMEOUT
#define	CBUSABORT_BIT_UNDEFINED_OPCODE  BIT_CBUS_MSC_UNDEF_CMD


#define	CBUSSTATUS_BIT_CONNECTED		BIT_CBUS_HPD
#define	CBUSABORT_BIT_PEER_ABORTED		BIT_CBUS_MSC_MT_ABRT_BY_PEER

#define REG_CBUS_PRI_START              (TX_PAGE_CBUS | 0x0012)
#define BIT_CBUS_MSC_PEER_CMD       BIT0
#define BIT_CBUS_MSC_MSG            BIT1
#define BIT_CBUS_MSC_READ_DEVCAP    BIT2
#define BIT_CBUS_MSC_WRITE_STAT     BIT3
#define BIT_CBUS_MSC_WRITE_BURST    BIT4

#define	MSC_START_BIT_MSC_CMD		        BIT_CBUS_MSC_PEER_CMD
#define	MSC_START_BIT_VS_CMD		        BIT_CBUS_MSC_MSG
#define	MSC_START_BIT_READ_REG		        BIT_CBUS_MSC_READ_DEVCAP
#define	MSC_START_BIT_WRITE_REG		        BIT_CBUS_MSC_WRITE_STAT
#define	MSC_START_BIT_WRITE_BURST	        BIT_CBUS_MSC_WRITE_BURST

#define REG_CBUS_PRI_ADDR_CMD           (TX_PAGE_CBUS | 0x0013)
#define REG_CBUS_PRI_WR_DATA_1ST        (TX_PAGE_CBUS | 0x0014)
#define REG_CBUS_PRI_WR_DATA_2ND        (TX_PAGE_CBUS | 0x0015)
#define REG_CBUS_PRI_RD_DATA_1ST        (TX_PAGE_CBUS | 0x0016)
#define REG_CBUS_PRI_RD_DATA_2ND        (TX_PAGE_CBUS | 0x0017)

#define REG_CBUS_PRI_VS_CMD             (TX_PAGE_CBUS | 0x0018)
#define REG_CBUS_PRI_VS_DATA            (TX_PAGE_CBUS | 0x0019)

#define	REG_CBUS_MSC_RETRY_INTERVAL		(TX_PAGE_CBUS | 0x001A)		/* default is 16 */
#define	REG_CBUS_DDC_FAIL_LIMIT			(TX_PAGE_CBUS | 0x001C)		/* default is 5  */
#define	REG_CBUS_MSC_FAIL_LIMIT			(TX_PAGE_CBUS | 0x001D)		/* default is 5  */

#define	REG_CBUS_MSC_INT2_STATUS        (TX_PAGE_CBUS | 0x001E)

#define BIT_CBUS_MSC_MR_WRITE_BURST     BIT0
#define BIT_CBUS_MSC_HB_MAX_FAIL        BIT1
#define BIT_CBUS_MSC_MR_SET_INT         BIT2
#define BIT_CBUS_MSC_MR_WRITE_STATE     BIT3
#define BIT_CBUS_MSC_PEER_STATE_CHG     BIT4

#define REG_CBUS_MSC_INT2_ENABLE        (TX_PAGE_CBUS | 0x001F)
#define BIT_CBUS_INTRP_MSC_MR_WRITE_BURST_EN     BIT0
#define BIT_CBUS_INTRP_MSC_HB_MAX_FAIL_EN        BIT1
#define BIT_CBUS_INTRP_MSC_MR_SET_INT_EN         BIT2
#define BIT_CBUS_INTRP_MSC_MR_WRITE_STATE_EN     BIT3
#define BIT_CBUS_INTRP_MSC_PEER_STATE_CHG_EN     BIT4

#define	REG_MSC_WRITE_BURST_LEN         (TX_PAGE_CBUS | 0x0020)       /* only for WRITE_BURST */
#define     MSC_MT_DONE_NACK_MASK       BIT6
#define     MSC_WRITE_BURST_LEN_MASK    0x0F

#define REG_MSC_TIMEOUT_LIMIT           (TX_PAGE_CBUS | 0x0022)
#define	MSC_TIMEOUT_LIMIT_MSB_MASK	        (0x0F)	        /* default is 1           */
#define	MSC_LEGACY_BIT					    (0x01 << 7)	    /* This should be cleared.*/

#define REG_CBUS_MSC_COMPATIBILITY_CTRL (TX_PAGE_CBUS | 0x002E)
#define BIT_CBUS_CEC_DISABLE            BIT4


#define	REG_CBUS_LINK_CONTROL_1			(TX_PAGE_CBUS | 0x0030)
#define BIT_CBUS_PACKET_LIMIT_MASK      0x1F

#define	REG_CBUS_LINK_CONTROL_2			(TX_PAGE_CBUS | 0x0031)
#define BIT_CBUS_INITIATOR_TIMEOUT_MASK 0x0C

#define	REG_CBUS_LINK_CONTROL_3			(TX_PAGE_CBUS | 0x0032)
#define	REG_CBUS_LINK_CONTROL_4			(TX_PAGE_CBUS | 0x0033)
#define	REG_CBUS_LINK_CONTROL_5			(TX_PAGE_CBUS | 0x0034)
#define	REG_CBUS_LINK_CONTROL_6			(TX_PAGE_CBUS | 0x0035)
#define	REG_CBUS_LINK_CONTROL_7			(TX_PAGE_CBUS | 0x0036)
#define REG_CBUS_LINK_STATUS_1          (TX_PAGE_CBUS | 0x0037)
#define REG_CBUS_LINK_STATUS_2          (TX_PAGE_CBUS | 0x0038)
#define	REG_CBUS_LINK_CONTROL_8			(TX_PAGE_CBUS | 0x0039)
#define	REG_CBUS_LINK_CONTROL_9			(TX_PAGE_CBUS | 0x003A)
#define	REG_CBUS_LINK_CONTROL_10		(TX_PAGE_CBUS | 0x003B)
#define	REG_CBUS_LINK_CONTROL_11		(TX_PAGE_CBUS | 0x003C)
#define	REG_CBUS_LINK_CONTROL_12		(TX_PAGE_CBUS | 0x003D)


#define REG_CBUS_LINK_CTRL9_0			(TX_PAGE_CBUS | 0x003A)
#define REG_CBUS_LINK_CTRL9_1           (TX_PAGE_CBUS | 0x00BA)

#define	REG_CBUS_DRV_STRENGTH_0			(TX_PAGE_CBUS | 0x0040)
#define	REG_CBUS_DRV_STRENGTH_1			(TX_PAGE_CBUS | 0x0041)
#define	REG_CBUS_ACK_CONTROL			(TX_PAGE_CBUS | 0x0042)
#define	REG_CBUS_CAL_CONTROL			(TX_PAGE_CBUS | 0x0043)	/* Calibration */

#define REG_CBUS_DEVICE_CAP_0           (TX_PAGE_CBUS | 0x0080)
#define REG_CBUS_DEVICE_CAP_1           (TX_PAGE_CBUS | 0x0081)
#define REG_CBUS_DEVICE_CAP_2           (TX_PAGE_CBUS | 0x0082)
#define REG_CBUS_DEVICE_CAP_3           (TX_PAGE_CBUS | 0x0083)
#define REG_CBUS_DEVICE_CAP_4           (TX_PAGE_CBUS | 0x0084)
#define REG_CBUS_DEVICE_CAP_5           (TX_PAGE_CBUS | 0x0085)
#define REG_CBUS_DEVICE_CAP_6           (TX_PAGE_CBUS | 0x0086)
#define REG_CBUS_DEVICE_CAP_7           (TX_PAGE_CBUS | 0x0087)
#define REG_CBUS_DEVICE_CAP_8           (TX_PAGE_CBUS | 0x0088)
#define REG_CBUS_DEVICE_CAP_9           (TX_PAGE_CBUS | 0x0089)
#define REG_CBUS_DEVICE_CAP_A           (TX_PAGE_CBUS | 0x008A)
#define REG_CBUS_DEVICE_CAP_B           (TX_PAGE_CBUS | 0x008B)
#define REG_CBUS_DEVICE_CAP_C           (TX_PAGE_CBUS | 0x008C)
#define REG_CBUS_DEVICE_CAP_D           (TX_PAGE_CBUS | 0x008D)
#define REG_CBUS_DEVICE_CAP_E           (TX_PAGE_CBUS | 0x008E)
#define REG_CBUS_DEVICE_CAP_F           (TX_PAGE_CBUS | 0x008F)

#define REG_CBUS_SET_INT_0				(TX_PAGE_CBUS | 0x00A0)
#define REG_CBUS_SET_INT_1				(TX_PAGE_CBUS | 0x00A1)
#define REG_CBUS_SET_INT_2				(TX_PAGE_CBUS | 0x00A2)
#define REG_CBUS_SET_INT_3				(TX_PAGE_CBUS | 0x00A3)

#define REG_CBUS_WRITE_STAT_0        	(TX_PAGE_CBUS | 0x00B0)
#define REG_CBUS_WRITE_STAT_1        	(TX_PAGE_CBUS | 0x00B1)
#define REG_CBUS_WRITE_STAT_2        	(TX_PAGE_CBUS | 0x00B2)
#define REG_CBUS_WRITE_STAT_3        	(TX_PAGE_CBUS | 0x00B3)


#define REG_CBUS_SCRATCHPAD_0           (TX_PAGE_CBUS | 0x00C0)

#define REG_CBUS_WRITE_STAT_ENABLE_0    (TX_PAGE_CBUS | 0x00E0)
#define REG_CBUS_WRITE_STAT_ENABLE_1    (TX_PAGE_CBUS | 0x00E1)
#define REG_CBUS_WRITE_STAT_ENABLE_2    (TX_PAGE_CBUS | 0x00E2)
#define REG_CBUS_WRITE_STAT_ENABLE_3    (TX_PAGE_CBUS | 0x00E3)

#define REG_CBUS_SET_INT_ENABLE_0    (TX_PAGE_CBUS | 0x00F0)
#define REG_CBUS_SET_INT_ENABLE_1    (TX_PAGE_CBUS | 0x00F1)
#define REG_CBUS_SET_INT_ENABLE_2    (TX_PAGE_CBUS | 0x00F2)
#define REG_CBUS_SET_INT_ENABLE_3    (TX_PAGE_CBUS | 0x00F3)
