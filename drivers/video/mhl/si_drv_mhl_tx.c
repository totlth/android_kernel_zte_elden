/*
 *	si_drv_mhl_tx.c <Firmware or Driver>
 *
 *  Copyright (c) 2002-2011, Silicon Image, Inc.  All rights reserved.
 *  No part of this work may be reproduced, modified, distributed, transmitted,
 *  transcribed, or translated into any language or computer format, in any form
 *  or by any means without written permission of: Silicon Image, Inc.,
 *  1060 East Arques Avenue, Sunnyvale, California 94085
*/

#include "si_cra.h"
#include "si_mhl_defs.h"
#include "si_mhl_tx_api.h"
#include "si_8334_regs.h"
#include "si_drv_mhl_tx.h"
#include "MHL_SiI8334.h"

#define SILICON_IMAGE_ADOPTER_ID 322
#define TRANSCODER_DEVICE_ID 0x8334

extern bool_t	vbusPowerState;
//
// Software power states are a little bit different than the hardware states but
// a close resemblance exists.
//
// D3 matches well with hardware state. In this state we receive RGND interrupts
// to initiate wake up pulse and device discovery
//
// Chip wakes up in D2 mode and interrupts MCU for RGND. Firmware changes the TX
// into D0 mode and sets its own operation mode as POWER_STATE_D0_NO_MHL because
// MHL connection has not yet completed.
//
// For all practical reasons, firmware knows only two states of hardware - D0 and D3.
//
// We move from POWER_STATE_D0_NO_MHL to POWER_STATE_D0_MHL only when MHL connection
// is established.
/*
//
//                             S T A T E     T R A N S I T I O N S
//
//
//                    POWER_STATE_D3                      POWER_STATE_D0_NO_MHL
//                   /--------------\                        /------------\
//                  /                \                      /     D0       \
//                 /                  \                \   /                \
//                /   DDDDDD  333333   \     RGND       \ /   NN  N  OOO     \
//                |   D     D     33   |-----------------|    N N N O   O     |
//                |   D     D  3333    |      IRQ       /|    N  NN  OOO      |
//                \   D     D      33  /               /  \                  /
//                 \  DDDDDD  333333  /                    \   CONNECTION   /
//                  \                /\                     /\             /
//                   \--------------/  \  TIMEOUT/         /  -------------
//                         /|\          \-------/---------/        ||
//                        / | \            500ms\                  ||
//                          |                     \                ||
//                          |  RSEN_LOW                            || MHL_EST
//                           \ (STATUS)                            ||  (IRQ)
//                            \                                    ||
//                             \      /------------\              //
//                              \    /              \            //
//                               \  /                \          //
//                                \/                  \ /      //
//                                 |    CONNECTED     |/======//    
//                                 |                  |\======/
//                                 \   (OPERATIONAL)  / \
//                                  \                /
//                                   \              /
//                                    \-----------/
//                                   POWER_STATE_D0_MHL
//
//
//
*/
#define	POWER_STATE_D3				3
#define	POWER_STATE_D0_NO_MHL		2
#define	POWER_STATE_D0_MHL			0
#define	POWER_STATE_FIRST_INIT		0xFF

#define	TIMER_FOR_MONITORING				(TIMER_0)

#define TX_HW_RESET_PERIOD		10	// 10 ms.
#define TX_HW_RESET_DELAY			100

//
// To remember the current power state.
//
static	uint8_t	fwPowerState = POWER_STATE_FIRST_INIT;

//
// To serialize the RCP commands posted to the CBUS engine, this flag
// is maintained by the function SiiMhlTxDrvSendCbusCommand()
//
static	bool_t		mscCmdInProgress;	// false when it is okay to send a new command
//
// Preserve Downstream HPD status
//
static	uint8_t	dsHpdStatus = 0;


#define	SET_BIT(offset,bitnumber)		SiiRegModify(offset,(1<<bitnumber),(1<<bitnumber))
#define	CLR_BIT(offset,bitnumber)		SiiRegModify(offset,(1<<bitnumber),0x00)
//
//
#define	DISABLE_DISCOVERY				SiiRegModify(REG_DISC_CTRL1,BIT0,0);
#define	ENABLE_DISCOVERY				SiiRegModify(REG_DISC_CTRL1,BIT0,BIT0);

#define STROBE_POWER_ON					SiiRegModify(REG_DISC_CTRL1,BIT1,0);
/*
    Look for interrupts on INTR1
    6 - MDI_HPD  - downstream hotplug detect (DSHPD)
*/

#define INTR_1_DESIRED_MASK   (BIT6)
#define	UNMASK_INTR_1_INTERRUPTS		SiiRegWrite(REG_INTR1_MASK, INTR_1_DESIRED_MASK)
#define	MASK_INTR_1_INTERRUPTS			SiiRegWrite(REG_INTR1_MASK, 0x00)
//
//	Look for interrupts on INTR4 (Register 0x74)
//		7 = RSVD		(reserved)
//		6 = RGND Rdy	(interested)
//		5 = MHL DISCONNECT	(interested)	
//		4 = CBUS LKOUT	(interested)
//		3 = USB EST		(interested)
//		2 = MHL EST		(interested)
//		1 = RPWR5V Change	(ignore)
//		0 = SCDT Change	(only if necessary)
//

#define	INTR_4_DESIRED_MASK				(BIT0 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6)
#define	UNMASK_INTR_4_INTERRUPTS		SiiRegWrite(REG_INTR4_MASK, INTR_4_DESIRED_MASK)
#define	MASK_INTR_4_INTERRUPTS			SiiRegWrite(REG_INTR4_MASK, 0x00)

//	Look for interrupts on INTR_5 (Register ??)
//		4 = FIFO UNDERFLOW	(interested)
//		3 = FIFO OVERFLOW	(interested)

#define	INTR_5_DESIRED_MASK				(BIT3 | BIT4)
#define	UNMASK_INTR_5_INTERRUPTS		SiiRegWrite(REG_INTR5_MASK, INTR_5_DESIRED_MASK)
#define	MASK_INTR_5_INTERRUPTS			SiiRegWrite(REG_INTR5_MASK, 0x00)

//	Look for interrupts on CBUS:CBUS_INTR_STATUS [0xC8:0x08]
//		7 = RSVD			(reserved)
//		6 = MSC_RESP_ABORT	(interested)
//		5 = MSC_REQ_ABORT	(interested)
//		4 = MSC_REQ_DONE	(interested)
//		3 = MSC_MSG_RCVD	(interested)
//		2 = DDC_ABORT		(interested)
//		1 = RSVD			(reserved)
//		0 = rsvd			(reserved)
#define	INTR_CBUS1_DESIRED_MASK			(BIT_CBUS_INTRP_DDC_ABRT_EN| BIT_CBUS_INTRP_MSC_MR_MSC_MSG_EN| BIT_CBUS_INTRP_MSC_MT_DONE_EN| BIT_CBUS_INTRP_MSC_MT_ABRT_EN | BIT_CBUS_INTRP_MSC_MR_ABRT_EN)
#define	UNMASK_CBUS1_INTERRUPTS			SiiRegWrite(REG_CBUS_INTR_ENABLE, INTR_CBUS1_DESIRED_MASK)
#define	MASK_CBUS1_INTERRUPTS			SiiRegWrite(REG_CBUS_INTR_ENABLE, 0x00)

#define	INTR_CBUS2_DESIRED_MASK			(BIT_CBUS_INTRP_MSC_MR_SET_INT_EN| BIT_CBUS_INTRP_MSC_MR_WRITE_STATE_EN)
#define	UNMASK_CBUS2_INTERRUPTS			SiiRegWrite(REG_CBUS_MSC_INT2_ENABLE, INTR_CBUS2_DESIRED_MASK)
#define	MASK_CBUS2_INTERRUPTS			SiiRegWrite(REG_CBUS_MSC_INT2_ENABLE, 0x00)

#define I2C_INACCESSIBLE -1
#define I2C_ACCESSIBLE 1

//
// Local scope functions.
//
static void Int1Isr (void);
static int  Int4Isr (void);
static void Int5Isr (void);
static void MhlCbusIsr (void);

static void CbusReset (void);
static void SwitchToD0 (void);
static void SwitchToD3 (void);
static void WriteInitialRegisterValues (void);
static void InitCBusRegs (void);
static void ForceUsbIdSwitchOpen (void);
static void ReleaseUsbIdSwitchOpen (void);
static void MhlTxDrvProcessConnection (void);
static void MhlTxDrvProcessDisconnection (void);

bool_t tmdsPoweredUp;
bool_t mhlConnected;
uint8_t g_chipRevId;

static void ProcessScdtStatusChange (void);

///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxDrvAcquireUpstreamHPDControl
//
// Acquire the direct control of Upstream HPD.
//
void SiiMhlTxDrvAcquireUpstreamHPDControl (void)
{
	// set reg_hpd_out_ovr_en to first control the hpd
	SET_BIT(REG_INT_CTRL, 4);
	TX_DEBUG_PRINT(("Drv: Upstream HPD Acquired.\n"));
}

///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxDrvAcquireUpstreamHPDControlDriveLow
//
// Acquire the direct control of Upstream HPD.
//
void SiiMhlTxDrvAcquireUpstreamHPDControlDriveLow (void)
{
	// set reg_hpd_out_ovr_en to first control the hpd and clear reg_hpd_out_ovr_val
 	SiiRegModify(REG_INT_CTRL, BIT5 | BIT4, BIT4);	// Force upstream HPD to 0 when not in MHL mode.
	TX_DEBUG_PRINT(("Drv: Upstream HPD Acquired - driven low.\n"));
}

///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxDrvReleaseUpstreamHPDControl
//
// Release the direct control of Upstream HPD.
//
void SiiMhlTxDrvReleaseUpstreamHPDControl (void)
{
   	// Un-force HPD (it was kept low, now propagate to source
	// let HPD float by clearing reg_hpd_out_ovr_en
   	CLR_BIT(REG_INT_CTRL, 4);
	TX_DEBUG_PRINT(("Drv: Upstream HPD released.\n"));
}

static void Int1Isr(void)
{
uint8_t regIntr1;
    regIntr1 = SiiRegRead(REG_INTR1);
    if (regIntr1)
    {
        // Clear all interrupts coming from this register.
        SiiRegWrite(REG_INTR1,regIntr1);

        if (BIT6 & regIntr1)
        {
        uint8_t cbusStatus;
        	//
        	// Check if a SET_HPD came from the downstream device.
        	//
        	cbusStatus = SiiRegRead(REG_PRI_XFR_ABORT_REASON);

        	// CBUS_HPD status bit
        	if(BIT6 & (dsHpdStatus ^ cbusStatus))
        	{
            uint8_t status = cbusStatus & BIT6;
        		TX_DEBUG_PRINT(("Drv: Downstream HPD changed to: %02X\n", (int) cbusStatus));

        		// Inform upper layer of change in Downstream HPD
        		SiiMhlTxNotifyDsHpdChange( status );

                if (status)
                {
                    SiiMhlTxDrvReleaseUpstreamHPDControl();  // this triggers an EDID read if control has not yet been released
                }

        		// Remember
        		dsHpdStatus = cbusStatus;
        	}
        }
    }
}


////////////////////////////////////////////////////////////////////
//
// E X T E R N A L L Y    E X P O S E D   A P I    F U N C T I O N S
//
////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxChipInitialize
//
// Chip specific initialization.
// This function is for SiI 8332/8336 Initialization: HW Reset, Interrupt enable.
//
//////////////////////////////////////////////////////////////////////////////

bool_t SiiMhlTxChipInitialize (void)
{
	tmdsPoweredUp = false;
	mhlConnected = false;
	mscCmdInProgress = false;	// false when it is okay to send a new command
	dsHpdStatus = 0;
	fwPowerState = POWER_STATE_FIRST_INIT;
    //SI_OS_DISABLE_DEBUG_CHANNEL(SII_OSAL_DEBUG_SCHEDULER);

    g_chipRevId = SiiRegRead(REG_DEV_REV);

	// then wait 100ms per MHL spec
	HalTimerWait(TX_HW_RESET_DELAY);
	TX_DEBUG_PRINT(("Drv: SiiMhlTxChipInitialize: %02X%02X%02x\n",
						g_chipRevId,
					SiiRegRead(TX_PAGE_L0 | 0x03),
					SiiRegRead(TX_PAGE_L0 | 0x02)));

	// setup device registers. Ensure RGND interrupt would happen.
	WriteInitialRegisterValues();

    //SiiOsMhlTxInterruptEnable();

	// CBUS interrupts are unmasked after performing the reset.
	// UNMASK_CBUS1_INTERRUPTS;
	// UNMASK_CBUS2_INTERRUPTS;

	//
	// Allow regular operation - i.e. pinAllowD3 is high so we do enter
	// D3 first time. Later on, SiIMon shall control this GPIO.
	//
	//pinAllowD3 = 1;

	SwitchToD3();

	return true;
}
///////////////////////////////////////////////////////////////////////////////
// SiiMhlTxDeviceIsr
//
// This function must be called from a master interrupt handler or any polling
// loop in the host software if during initialization call the parameter
// interruptDriven was set to true. SiiMhlTxGetEvents will not look at these
// events assuming firmware is operating in interrupt driven mode. MhlTx component
// performs a check of all its internal status registers to see if a hardware event
// such as connection or disconnection has happened or an RCP message has been
// received from the connected device. Due to the interruptDriven being true,
// MhlTx code will ensure concurrency by asking the host software and hardware to
// disable interrupts and restore when completed. Device interrupts are cleared by
// the MhlTx component before returning back to the caller. Any handling of
// programmable interrupt controller logic if present in the host will have to
// be done by the caller after this function returns back.

// This function has no parameters and returns nothing.
//
// This is the master interrupt handler for 9244. It calls sub handlers
// of interest. Still couple of status would be required to be picked up
// in the monitoring routine (Sii9244TimerIsr)
//
// To react in least amount of time hook up this ISR to processor's
// interrupt mechanism.
//
// Just in case environment does not provide this, set a flag so we
// call this from our monitor (Sii9244TimerIsr) in periodic fashion.
//
// Device Interrupts we would look at
//		RGND		= to wake up from D3
//		MHL_EST 	= connection establishment
//		CBUS_LOCKOUT= Service USB switch
//		CBUS 		= responder to peer messages
//					  Especially for DCAP etc time based events
//
void SiiMhlTxDeviceIsr (void)
{
	uint8_t intMStatus, i; //master int status
	//
	// Look at discovery interrupts if not yet connected.
	//

	i=0;

	do
	{
		if( POWER_STATE_D0_MHL != fwPowerState )
		{
			//
			// Check important RGND, MHL_EST, CBUS_LOCKOUT and SCDT interrupts
			// During D3 we only get RGND but same ISR can work for both states
			//
			if (I2C_INACCESSIBLE == Int4Isr())
			{
				return; // don't do any more I2C traffic until the next interrupt.
			}
		}
		else if( POWER_STATE_D0_MHL == fwPowerState )
		{

			if (I2C_INACCESSIBLE == Int4Isr())
			{
				return; // don't do any more I2C traffic until the next interrupt.
			}

			if(POWER_STATE_D0_MHL == fwPowerState)
			{
				
#if 0
				TX_DEBUG_PRINT(("********* EXITING ISR *************\n"));
				TX_DEBUG_PRINT(("Drv: INT1 Status = %02X\n", (int) SiiRegRead((TX_PAGE_L0 | 0x0071))));
				TX_DEBUG_PRINT(("Drv: INT2 Status = %02X\n", (int) SiiRegRead((TX_PAGE_L0 | 0x0072))));
				TX_DEBUG_PRINT(("Drv: INT3 Status = %02X\n", (int) SiiRegRead((TX_PAGE_L0 | 0x0073))));
				TX_DEBUG_PRINT(("Drv: INT4 Status = %02X\n", (int) SiiRegRead((TX_PAGE_L0 | 0x0073))));
				TX_DEBUG_PRINT(("Drv: INT5 Status = %02X\n", (int) SiiRegRead(REG_INTR4)));
				TX_DEBUG_PRINT(("Drv: INT5 Status = %02X\n", (int) SiiRegRead(REG_INTR5)));
			
			
				TX_DEBUG_PRINT(("Drv: cbusInt Status = %02X\n", (int) SiiRegRead(TX_PAGE_CBUS | 0x0008)));
			
				TX_DEBUG_PRINT(("Drv: CBUS INTR_2: 0x1E: %02X\n", (int) SiiRegRead(TX_PAGE_CBUS | 0x001E)));
				TX_DEBUG_PRINT(("Drv: A0 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00A0))));
				TX_DEBUG_PRINT(("Drv: A1 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00A1))));
				TX_DEBUG_PRINT(("Drv: A2 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00A2))));
				TX_DEBUG_PRINT(("Drv: A3 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00A3))));
			
				TX_DEBUG_PRINT(("Drv: B0 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00B0))));
				TX_DEBUG_PRINT(("Drv: B1 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00B1))));
				TX_DEBUG_PRINT(("Drv: B2 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00B2))));
				TX_DEBUG_PRINT(("Drv: B3 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00B3))));
			
				TX_DEBUG_PRINT(("Drv: E0 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00E0))));
				TX_DEBUG_PRINT(("Drv: E1 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00E1))));
				TX_DEBUG_PRINT(("Drv: E2 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00E2))));
				TX_DEBUG_PRINT(("Drv: E3 STATUS Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00E3))));
			
				TX_DEBUG_PRINT(("Drv: F0 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00F0))));
				TX_DEBUG_PRINT(("Drv: F1 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00F1))));
				TX_DEBUG_PRINT(("Drv: F2 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00F2))));
				TX_DEBUG_PRINT(("Drv: F3 INT Set = %02X\n", (int) SiiRegRead((TX_PAGE_CBUS | 0x00F3))));
				TX_DEBUG_PRINT(("********* END OF EXITING ISR *************\n"));
	#endif
				// If the Int4Isr handler didn't move the transmitter to D3 as the
				// result of a cable disconnection continue to check other interrupt
				// sources.
				Int5Isr();
	
				// Check for any peer messages for DCAP_CHG etc
				// Dispatch to have the CBUS module working only once connected.
				MhlCbusIsr();
				Int1Isr();
				
			}
		}
		if( POWER_STATE_D3 != fwPowerState )
    	{
    		// Call back into the MHL component to give it a chance to
    		// take care of any message processing caused by this interrupt.
    		MhlTxProcessEvents();
    	}
		intMStatus = SiiRegRead(REG_INTR_STATE);	// read status
		if(0xFF == intMStatus)
		{
			intMStatus = 0;
			TX_DEBUG_PRINT(("\nDrv: EXITING ISR DUE TO intMStatus - 0xFF loop = [%02X] intMStatus = [%02X] \n\n", (int) i, (int)intMStatus));
		}
		i++;

		intMStatus &= 0x01; //RG mask bit 0
	} while (intMStatus);
}


///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxDrvTmdsControl
//
// Control the TMDS output. MhlTx uses this to support RAP content on and off.
//
void SiiMhlTxDrvTmdsControl (bool_t enable)
{
	if( enable )
	{
		SET_BIT(REG_TMDS_CCTRL, 4);
	    TX_DEBUG_PRINT(("Drv: TMDS Output Enabled\n"));
        SiiMhlTxDrvReleaseUpstreamHPDControl();  // this triggers an EDID read
	}
	else
	{
		CLR_BIT(REG_TMDS_CCTRL, 4);
	    TX_DEBUG_PRINT(("Drv: TMDS Ouput Disabled\n"));
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxDrvNotifyEdidChange
//
// MhlTx may need to inform upstream device of an EDID change. This can be
// achieved by toggling the HDMI HPD signal or by simply calling EDID read
// function.
//
void SiiMhlTxDrvNotifyEdidChange (void)
{
    TX_DEBUG_PRINT(("Drv: SiiMhlTxDrvNotifyEdidChange\n"));
	//
	// Prepare to toggle HPD to upstream
	//
    SiiMhlTxDrvAcquireUpstreamHPDControl();

	// reg_hpd_out_ovr_val = LOW to force the HPD low
	CLR_BIT(REG_INT_CTRL, 5);

	// wait a bit
	HalTimerWait(110);

	// release HPD back to high by reg_hpd_out_ovr_val = HIGH
	SET_BIT(REG_INT_CTRL, 5);

    // release control to allow transcoder to modulate for CLR_HPD and SET_HPD
    SiiMhlTxDrvReleaseUpstreamHPDControl();
}
//------------------------------------------------------------------------------
// Function:    SiiMhlTxDrvSendCbusCommand
//
// Write the specified Sideband Channel command to the CBUS.
// Command can be a MSC_MSG command (RCP/RAP/RCPK/RCPE/RAPK), or another command 
// such as READ_DEVCAP, SET_INT, WRITE_STAT, etc.
//
// Parameters:  
//              pReq    - Pointer to a cbus_req_t structure containing the 
//                        command to write
// Returns:     true    - successful write
//              false   - write failed
//------------------------------------------------------------------------------

bool_t SiiMhlTxDrvSendCbusCommand (cbus_req_t *pReq)
{
    bool_t  success = true;

    uint8_t startbit;

	//
	// If not connected, return with error
	//
	if( (POWER_STATE_D0_MHL != fwPowerState ) || (mscCmdInProgress))
	{
	    TX_DEBUG_PRINT(("Error: Drv:%d fwPowerState: %02X, or CBUS(0x0A):%02X mscCmdInProgress = %d\n",(int)__LINE__,
			(int) fwPowerState,
			(int) SiiRegRead(REG_CBUS_BUS_STATUS),
			(int) mscCmdInProgress));

   		return false;
	}
	// Now we are getting busy
	mscCmdInProgress	= true;
#if 0
    TX_DEBUG_PRINT(("Drv: Sending MSC command %02X, %02X, %02X, %02X\n",
			(int)pReq->command,
			(int)(pReq->offsetData),
		 	(int)pReq->payload_u.msgData[0],
		 	(int)pReq->payload_u.msgData[1]));
#endif
    /****************************************************************************************/
    /* Setup for the command - write appropriate registers and determine the correct        */
    /*                         start bit.                                                   */
    /****************************************************************************************/

	// Set the offset and outgoing data byte right away
	SiiRegWrite(REG_CBUS_PRI_ADDR_CMD, pReq->offsetData); 	// set offset
	SiiRegWrite(REG_CBUS_PRI_WR_DATA_1ST, pReq->payload_u.msgData[0]);

    startbit = 0x00;
    switch ( pReq->command )
    {
		case MHL_SET_INT:	// Set one interrupt register = 0x60
			startbit = MSC_START_BIT_WRITE_REG;
			break;

        case MHL_WRITE_STAT:	// Write one status register = 0x60 | 0x80
            startbit = MSC_START_BIT_WRITE_REG;
            break;

        case MHL_READ_DEVCAP:	// Read one device capability register = 0x61
            startbit = MSC_START_BIT_READ_REG;
            break;

 		case MHL_GET_STATE:			// 0x62 -
		case MHL_GET_VENDOR_ID:		// 0x63 - for vendor id	
		case MHL_SET_HPD:			// 0x64	- Set Hot Plug Detect in follower
		case MHL_CLR_HPD:			// 0x65	- Clear Hot Plug Detect in follower
		case MHL_GET_SC1_ERRORCODE:		// 0x69	- Get channel 1 command error code
		case MHL_GET_DDC_ERRORCODE:		// 0x6A	- Get DDC channel command error code.
		case MHL_GET_MSC_ERRORCODE:		// 0x6B	- Get MSC command error code.
		case MHL_GET_SC3_ERRORCODE:		// 0x6D	- Get channel 3 command error code.
			SiiRegWrite(REG_CBUS_PRI_ADDR_CMD, pReq->command );
            startbit = MSC_START_BIT_MSC_CMD;
            break;

        case MHL_MSC_MSG:
			SiiRegWrite(REG_CBUS_PRI_WR_DATA_2ND, pReq->payload_u.msgData[1]);
			SiiRegWrite(REG_CBUS_PRI_ADDR_CMD, pReq->command );
            startbit = MSC_START_BIT_VS_CMD;
            break;

        case MHL_WRITE_BURST:
            SiiRegWrite(REG_MSC_WRITE_BURST_LEN, pReq->length -1 );

            // Now copy all bytes from array to local scratchpad
            if (NULL == pReq->payload_u.pdatabytes)
            {
                TX_DEBUG_PRINT(("\nDrv:%d Put pointer to WRITE_BURST data in req.pdatabytes!!!\n\n",(int)__LINE__));
				success = false;
            }
            else
            {
	            uint8_t *pData = pReq->payload_u.pdatabytes;
                TX_DEBUG_PRINT(("\nDrv: Writing data into scratchpad\n\n"));
                SiiRegWriteBlock(REG_CBUS_SCRATCHPAD_0,pData,pReq->length);
                startbit = MSC_START_BIT_WRITE_BURST;
			}
            break;

        default:
            success = false;
            break;
    }

    /****************************************************************************************/
    /* Trigger the CBUS command transfer using the determined start bit.                    */
    /****************************************************************************************/

    if ( success )
    {
        SiiRegWrite(REG_CBUS_PRI_START, startbit );
    }
    else
    {
        TX_DEBUG_PRINT(("\nDrv:%d SiiMhlTxDrvSendCbusCommand failed\n\n",(int)__LINE__));
    }

    return( success );
}

bool_t SiiMhlTxDrvCBusBusy (void)
{
    return mscCmdInProgress ? true : false;
}

///////////////////////////////////////////////////////////////////////////
// WriteInitialRegisterValues
//
//
///////////////////////////////////////////////////////////////////////////
static void WriteInitialRegisterValues (void)
{
	//TX_DEBUG_PRINT(("Drv: WriteInitialRegisterValues\n"));

	// Power Up
	SiiRegWrite(REG_DPD, 0x3F);			// Power up CVCC 1.2V core
	SiiRegWrite(REG_TMDS_CLK_EN, 0x01);			// Enable TxPLL Clock
	SiiRegWrite(REG_TMDS_CH_EN, 0x11);			// Enable Tx Clock Path & Equalizer

	SiiRegWrite(REG_MHLTX_CTL1, 0x10); // TX Source termination ON
	SiiRegWrite(REG_MHLTX_CTL6, 0xBC); // Enable 1X MHL clock output
	SiiRegWrite(REG_MHLTX_CTL2, 0x3C); // TX Differential Driver Config
	SiiRegWrite(REG_MHLTX_CTL4, 0xC8);
	SiiRegWrite(REG_MHLTX_CTL7, 0x03); // 2011-10-10
	SiiRegWrite(REG_MHLTX_CTL8, 0x0A); // PLL bias current, PLL BW Control

	// Analog PLL Control
	SiiRegWrite(REG_TMDS_CCTRL, 0x08);			// Enable Rx PLL clock 2011-10-10 - select BGR circuit for voltage references
	SiiRegWrite(REG_USB_CHARGE_PUMP, 0x8C);		// 2011-10-10 USB charge pump clock
    SiiRegWrite(REG_TMDS_CTRL4, 0x02);

	SiiRegWrite(REG_TMDS0_CCTRL2, 0x00);
	SiiRegModify(REG_DVI_CTRL3, BIT5, 0);      // 2011-10-10
	SiiRegWrite(REG_TMDS_TERMCTRL1, 0x60);

	SiiRegWrite(REG_PLL_CALREFSEL, 0x03);			// PLL Calrefsel
	SiiRegWrite(REG_PLL_VCOCAL, 0x20);			// VCO Cal
	SiiRegWrite(REG_EQ_DATA0, 0xE0);			// Auto EQ
	SiiRegWrite(REG_EQ_DATA1, 0xC0);			// Auto EQ
	SiiRegWrite(REG_EQ_DATA2, 0xA0);			// Auto EQ
	SiiRegWrite(REG_EQ_DATA3, 0x80);			// Auto EQ
	SiiRegWrite(REG_EQ_DATA4, 0x60);			// Auto EQ
	SiiRegWrite(REG_EQ_DATA5, 0x40);			// Auto EQ
	SiiRegWrite(REG_EQ_DATA6, 0x20);			// Auto EQ
	SiiRegWrite(REG_EQ_DATA7, 0x00);			// Auto EQ

	SiiRegWrite(REG_BW_I2C, 0x0A);			// Rx PLL BW ~ 4MHz
	SiiRegWrite(REG_EQ_PLL_CTRL1, 0x06);			// Rx PLL BW value from I2C

	SiiRegWrite(REG_MON_USE_COMP_EN, 0x06);

    // synchronous s/w reset
	SiiRegWrite(REG_ZONE_CTRL_SW_RST, 0x60);			// Manual zone control
	SiiRegWrite(REG_ZONE_CTRL_SW_RST, 0xE0);			// Manual zone control

	SiiRegWrite(REG_MODE_CONTROL, 0x00);			// PLL Mode Value

	SiiRegWrite(REG_SYS_CTRL1, 0x35);			// bring out from power down (script moved this here from above)

	SiiRegWrite(REG_DISC_CTRL2, 0xAD);
	SiiRegWrite(REG_DISC_CTRL5, 0x57);				// 1.8V CBUS VTH 5K pullup for MHL state
	SiiRegWrite(REG_DISC_CTRL6, 0x11);				// RGND & single discovery attempt (RGND blocking)
	SiiRegWrite(REG_DISC_CTRL8, 0x82);				// Ignore VBUS
	SiiRegWrite(REG_DISC_CTRL9, 0x24);				// No OTG, Discovery pulse proceed, Wake pulse not bypassed
	SiiRegWrite(REG_DISC_CTRL4, 0x8C);				// Pull-up resistance off for IDLE state.
	SiiRegWrite(REG_DISC_CTRL1, 0x27);				// Enable CBUS discovery
	SiiRegWrite(REG_DISC_CTRL7, 0x20);				// use 1K only setting
	SiiRegWrite(REG_DISC_CTRL3, 0x86);				// MHL CBUS discovery

	CLR_BIT(REG_INT_CTRL, 6);//change hpd out pin from defult open-drain to push-pull by garyyuan
	if (fwPowerState != TX_POWER_STATE_D3) {			// Don't force HPD to 0 during wake-up from D3
		SiiRegModify(REG_INT_CTRL, BIT5 | BIT4, BIT4);	// Force HPD to 0 when not in MHL mode.
		}

	SiiRegWrite(REG_SRST, 0x84); 					// Enable Auto soft reset on SCDT = 0

	SiiRegWrite(REG_DCTL, 0x1C); 		// HDMI Transcode mode enable

	CbusReset();

	InitCBusRegs();
}

///////////////////////////////////////////////////////////////////////////
// InitCBusRegs
//
///////////////////////////////////////////////////////////////////////////
static void InitCBusRegs (void)
{

	TX_DEBUG_PRINT(("Drv: InitCBusRegs\n"));

	SiiRegWrite(REG_CBUS_COMMON_CONFIG, 0xF2); 			// Increase DDC translation layer timer
	SiiRegWrite(REG_CBUS_LINK_CONTROL_7, 0x0B); 			// Drive High Time. -- changed from 0x03 on 2011-11-21 -- changed from 0x0C on 2011-10-03 - 17:00
	SiiRegWrite(REG_CBUS_LINK_CONTROL_8, 0x30); 		// Use programmed timing.
	SiiRegWrite(REG_CBUS_DRV_STRENGTH_0, 0x03); 			// CBUS Drive Strength

#define DEVCAP_REG(x) (REG_CBUS_DEVICE_CAP_0 | DEVCAP_OFFSET_##x)
	// Setup our devcap
	SiiRegWrite(DEVCAP_REG(DEV_STATE      ) ,DEVCAP_VAL_DEV_STATE       );
	SiiRegWrite(DEVCAP_REG(MHL_VERSION    ) ,DEVCAP_VAL_MHL_VERSION     );
	SiiRegWrite(DEVCAP_REG(DEV_CAT        ) ,DEVCAP_VAL_DEV_CAT         );
	SiiRegWrite(DEVCAP_REG(ADOPTER_ID_H   ) ,DEVCAP_VAL_ADOPTER_ID_H    );
	SiiRegWrite(DEVCAP_REG(ADOPTER_ID_L   ) ,DEVCAP_VAL_ADOPTER_ID_L    );
	SiiRegWrite(DEVCAP_REG(VID_LINK_MODE  ) ,DEVCAP_VAL_VID_LINK_MODE   );
	SiiRegWrite(DEVCAP_REG(AUD_LINK_MODE  ) ,DEVCAP_VAL_AUD_LINK_MODE   );
	SiiRegWrite(DEVCAP_REG(VIDEO_TYPE     ) ,DEVCAP_VAL_VIDEO_TYPE      );
	SiiRegWrite(DEVCAP_REG(LOG_DEV_MAP    ) ,DEVCAP_VAL_LOG_DEV_MAP     );
	SiiRegWrite(DEVCAP_REG(BANDWIDTH      ) ,DEVCAP_VAL_BANDWIDTH       );
	SiiRegWrite(DEVCAP_REG(FEATURE_FLAG   ) ,DEVCAP_VAL_FEATURE_FLAG    );
	SiiRegWrite(DEVCAP_REG(DEVICE_ID_H    ) ,DEVCAP_VAL_DEVICE_ID_H     );
	SiiRegWrite(DEVCAP_REG(DEVICE_ID_L    ) ,DEVCAP_VAL_DEVICE_ID_L     );
	SiiRegWrite(DEVCAP_REG(SCRATCHPAD_SIZE) ,DEVCAP_VAL_SCRATCHPAD_SIZE );
	SiiRegWrite(DEVCAP_REG(INT_STAT_SIZE  ) ,DEVCAP_VAL_INT_STAT_SIZE   );
	SiiRegWrite(DEVCAP_REG(RESERVED       ) ,DEVCAP_VAL_RESERVED        );

	// Make bits 2,3 (initiator timeout) to 1,1 for register CBUS_LINK_CONTROL_2
    SiiRegModify(REG_CBUS_LINK_CONTROL_2,BIT_CBUS_INITIATOR_TIMEOUT_MASK,BIT_CBUS_INITIATOR_TIMEOUT_MASK);

	 // Clear legacy bit on Wolverine TX. and set timeout to 0xF
    SiiRegWrite(REG_MSC_TIMEOUT_LIMIT, 0x0F);

	// Set NMax to 1
	SiiRegWrite(REG_CBUS_LINK_CONTROL_1, 0x01);
    SiiRegModify(REG_CBUS_MSC_COMPATIBILITY_CTRL, BIT_CBUS_CEC_DISABLE,BIT_CBUS_CEC_DISABLE);  // disallow vendor specific commands

}

///////////////////////////////////////////////////////////////////////////
//
// ForceUsbIdSwitchOpen
//
///////////////////////////////////////////////////////////////////////////
static void ForceUsbIdSwitchOpen (void)
{
	DISABLE_DISCOVERY
	SiiRegModify(REG_DISC_CTRL6, BIT6, BIT6);				// Force USB ID switch to open
	SiiRegWrite(REG_DISC_CTRL3, 0x86);
	SiiRegModify(REG_INT_CTRL, BIT5 | BIT4, BIT4);		// Force HPD to 0 when not in Mobile HD mode.
}
///////////////////////////////////////////////////////////////////////////
//
// ReleaseUsbIdSwitchOpen
//
///////////////////////////////////////////////////////////////////////////
static void ReleaseUsbIdSwitchOpen (void)
{
	HalTimerWait(50); // per spec
	SiiRegModify(REG_DISC_CTRL6, BIT6, 0x00);
	ENABLE_DISCOVERY
}
/*
	SiiMhlTxDrvProcessMhlConnection
		optionally called by the MHL Tx Component after giving the OEM layer the
		first crack at handling the event.
*/
void SiiMhlTxDrvProcessRgndMhl( void )
{
	SiiRegModify(REG_DISC_CTRL9, BIT0, BIT0);
}
///////////////////////////////////////////////////////////////////////////
// ProcessRgnd
//
// H/W has detected impedance change and interrupted.
// We look for appropriate impedance range to call it MHL and enable the
// hardware MHL discovery logic. If not, disable MHL discovery to allow
// USB to work appropriately.
//
// In current chip a firmware driven slow wake up pulses are sent to the
// sink to wake that and setup ourselves for full D0 operation.
///////////////////////////////////////////////////////////////////////////
void ProcessRgnd (void)
{
	uint8_t rgndImpedance;
	//
	// Impedance detection has completed - process interrupt
	//
	rgndImpedance = SiiRegRead(REG_DISC_STAT2) & 0x03;
	TX_DEBUG_PRINT(("Drv: RGND = %02X : \n", (int)rgndImpedance));

	//
	// 00, 01 or 11 means USB.
	// 10 means 1K impedance (MHL)
	//
	// If 1K, then only proceed with wake up pulses
	if (0x02 == rgndImpedance)
	{
		TX_DEBUG_PRINT(("(MHL Device)\n"));
/*ycm 20120801 delete for vsense*/
//		SiiMhlTxNotifyRgndMhl(); // this will call the application and then optionally call
/***********/
		//The sequence of events during MHL discovery is as follows:
		//	(i) SiI9244 blocks on RGND interrupt (Page0:0x74[6]).
		//	(ii) System firmware turns off its own VBUS if present.
		//	(iii) System firmware waits for about 200ms (spec: TVBUS_CBUS_STABLE, 100 - 1000ms), then checks for the presence of
		//		VBUS from the Sink.
		//	(iv) If VBUS is present then system firmware proceed to drive wake pulses to the Sink as described in previous
		//		section.
		//	(v) If VBUS is absent the system firmware turns on its own VBUS, wait for an additional 200ms (spec:
		//		TVBUS_OUT_TO_STABLE, 100 - 1000ms), and then proceed to drive wake pulses to the Sink as described in above.

		// AP need to check VBUS power present or absent in here 	// by oscar 20110527
		
#if (VBUS_POWER_CHK == ENABLE)			// Turn on VBUS output.
		AppVbusControl( vbusPowerState = false );
#endif

		TX_DEBUG_PRINT(("[MHL]: Waiting T_SRC_VBUS_CBUS_TO_STABLE (%d ms)\n", (int)T_SRC_VBUS_CBUS_TO_STABLE));
		//HalTimerWait(T_SRC_VBUS_CBUS_TO_STABLE);
	}
	else
	{
		SiiRegModify(REG_DISC_CTRL9, BIT3, BIT3);	// USB Established
		TX_DEBUG_PRINT(("(Non-MHL Device)\n"));
	}
}


////////////////////////////////////////////////////////////////////
// SwitchToD0
// This function performs s/w as well as h/w state transitions.
//
// Chip comes up in D2. Firmware must first bring it to full operation
// mode in D0.
////////////////////////////////////////////////////////////////////
void SwitchToD0 (void)
{
	TX_DEBUG_PRINT(("Drv: Switch to D0\n"));
//	TX_DEBUG_PRINT(("[%d] Drv: Switch To Full power mode (D0)\n",
//							(int) (HalTimerElapsed( ELAPSED_TIMER ) * MONITORING_PERIOD)) );

	//
	// WriteInitialRegisterValues switches the chip to full power mode.
	//
	WriteInitialRegisterValues();

	// Force Power State to ON

	STROBE_POWER_ON // Force Power State to ON

	SiiRegModify(TPI_DEVICE_POWER_STATE_CTRL_REG, TX_POWER_STATE_MASK, 0x00);

	fwPowerState = POWER_STATE_D0_NO_MHL;
}
////////////////////////////////////////////////////////////////////
// SwitchToD3
//
// This function performs s/w as well as h/w state transitions.
//
////////////////////////////////////////////////////////////////////
extern void CBusQueueReset(void);

void SwitchToD3 (void)
{
	if(POWER_STATE_D3 != fwPowerState)
	{
		TX_DEBUG_PRINT(("Drv: Switch To D3\n"));
		//SiiRegModify(REG_DISC_CTRL6,  BIT4, BIT4); //Block RGND INT in Discovery SM. added by garyyuan 20100804
//		TX_DEBUG_PRINT(("[%d] Drv: Switch To D3: pinAllowD3 = %d\n",
//							(int) (HalTimerElapsed( ELAPSED_TIMER ) * MONITORING_PERIOD), (int) pinAllowD3 ) );

		//pinM2uVbusCtrlM = 1;
		//pinMhlConn = 1;
		//pinUsbConn = 0;

		ForceUsbIdSwitchOpen();
		HalTimerWait(50);

		//
		// To allow RGND engine to operate correctly.
		// So when moving the chip from D0 MHL connected to D3 the values should be
		// 94[1:0] = 00  reg_cbusmhl_pup_sel[1:0] should be set for open
		// 93[7:6] = 00  reg_cbusdisc_pup_sel[1:0] should be set for open
		// 93[5:4] = 00  reg_cbusidle_pup_sel[1:0] = open (default)
		//
		// Disable CBUS pull-up during RGND measurement
//		I2C_WriteByte(PAGE_0_0X72, 0x93, 0x04);
//		ReadModifyWritePage0(0x93, BIT_7 | BIT_6 | BIT_5 | BIT_4, 0);

//		ReadModifyWritePage0(0x94, BIT_1 | BIT_0, 0);

		// 1.8V CBUS VTH & GND threshold
//		I2C_WriteByte(PAGE_0_0X72, 0x94, 0x64);

		ReleaseUsbIdSwitchOpen();



		// Force HPD to 0 when not in MHL mode.
        SiiMhlTxDrvAcquireUpstreamHPDControlDriveLow();

		// Change TMDS termination to high impedance on disconnection; reduce leakage to QCOM HDMI out, due to QCOM HDMI out can't be truly tri-stated. Each TMDS pin has about 0.1125mA leakage from AVCC33.
		// Bits 1:0 set to 11


		SiiRegWrite(REG_MHLTX_CTL1, 0xD0);

		//
		// GPIO controlled from SiIMon can be utilized to disallow
		// low power mode, thereby allowing SiIMon to debug register contents.
		// Otherwise SiIMon reads all registers as 0xFF
		//
//		if(pinAllowD3)
//		{
			// wait Tsrc:cbus_float
			HalTimerWait(50);
			//
			// Change state to D3 by clearing bit 0 of 3D (SW_TPI, Page 1) register
			// ReadModifyWriteIndexedRegister(INDEXED_PAGE_1, 0x3D, BIT_0, 0x00);
			//
			CLR_BIT(TX_PAGE_L1 | 0x003D, 0);
			
			CBusQueueReset();

			fwPowerState = POWER_STATE_D3;
//		}

#if (VBUS_POWER_CHK == ENABLE)		// Turn VBUS power off when switch to D3(cable out)
		if( vbusPowerState == false )
		{
			AppVbusControl( vbusPowerState = true );
		}
#endif
	}
	else
	{
		fwPowerState = POWER_STATE_D0_NO_MHL;
	}

}


////////////////////////////////////////////////////////////////////
// Int4Isr
//
//
//	Look for interrupts on INTR4 (Register 0x74)
//		7 = RSVD		(reserved)
//		6 = RGND Rdy	(interested)
//		5 = VBUS Low	(ignore)	
//		4 = CBUS LKOUT	(interested)
//		3 = USB EST		(interested)
//		2 = MHL EST		(interested)
//		1 = RPWR5V Change	(ignore)
//		0 = SCDT Change	(interested during D0)
////////////////////////////////////////////////////////////////////
static int Int4Isr (void)
{
	uint8_t int4Status;

	int4Status = SiiRegRead(REG_INTR4);	// read status
	if( int4Status )
	{
	    TX_DEBUG_PRINT(("Drv: int4Status: 0x%02X\n", (int) int4Status));
	}

	// When I2C is inoperational (D3) and a previous interrupt brought us here, do nothing.
	if(0xFF == int4Status || 0x87 == int4Status|| 0x38 == int4Status || 0x95 == int4Status)
	{
		return I2C_INACCESSIBLE;
	}

	if((int4Status & BIT0)&&( POWER_STATE_D0_MHL == fwPowerState )) // SCDT Status Change
	{
		if (g_chipRevId < 1)
			ProcessScdtStatusChange();
	}

	// process MHL_EST interrupt
	if(int4Status & BIT2) // MHL_EST_INT
	{
		MhlTxDrvProcessConnection();
	}

	// process USB_EST interrupt
	else if(int4Status & BIT3)
	{
		TX_DEBUG_PRINT(("Drv: uUSB-A type device detected.\n"));
		SiiRegWrite(REG_DISC_STAT2, 0x80);	// Exit D3 via CBUS falling edge
		SwitchToD3();
		return I2C_INACCESSIBLE;
	}

	if (int4Status & BIT5)
	{
		MhlTxDrvProcessDisconnection();
		return I2C_INACCESSIBLE;
	}

	if((POWER_STATE_D0_MHL != fwPowerState) && (int4Status & BIT6))
	{
		// Switch to full power mode.
		SwitchToD0();

	//
	// If a sink is connected but not powered on, this interrupt can keep coming
	// Determine when to go back to sleep. Say after 1 second of this state.
	//
	// Check RGND register and send wake up pulse to the peer
	//
	ProcessRgnd();
	}

	// Can't succeed at these in D3

	if(fwPowerState != POWER_STATE_D3)
	{
		// CBUS Lockout interrupt?
		if (int4Status & BIT4)
		{
			TX_DEBUG_PRINT(("Drv: CBus Lockout\n"));
			
			SwitchToD0();
			ForceUsbIdSwitchOpen();
			ReleaseUsbIdSwitchOpen();
			SwitchToD3();
		}
    }
	SiiRegWrite(REG_INTR4, int4Status); // clear all interrupts
	return I2C_ACCESSIBLE;
}

////////////////////////////////////////////////////////////////////
// Int5Isr
//
//
//	Look for interrupts on INTR5
//		7 = 
//		6 = 
//		5 = 
//		4 = 
//		3 = 
//		2 = 
//		1 = 
//		0 = 
////////////////////////////////////////////////////////////////////
static void Int5Isr (void)
{
	uint8_t int5Status;

	int5Status = SiiRegRead(REG_INTR5);	// read status
#if 0
	if((int5Status & BIT4) || (int5Status & BIT3)) // FIFO U/O
	{
		TX_DEBUG_PRINT(("** int5Status = %02X; Applying MHL FIFO Reset\n", (int)int5Status));
		SiiRegWrite(REG_SRST, 0x94);
		SiiRegWrite(REG_SRST, 0x84);
	}
#endif
	SiiRegWrite(REG_INTR5, int5Status);	// clear all interrupts
}

extern void mhl_plug_unplug(int state);
///////////////////////////////////////////////////////////////////////////
//
// MhlTxDrvProcessConnection
//
///////////////////////////////////////////////////////////////////////////
static void MhlTxDrvProcessConnection (void)
{

	TX_DEBUG_PRINT (("Drv: MHL Cable Connected. CBUS:0x0A = %02X\n", (int) SiiRegRead(TX_PAGE_CBUS | 0x000A)));

	if( POWER_STATE_D0_MHL == fwPowerState )
	{
		return;
	}
	// VBUS control gpio
	//pinM2uVbusCtrlM = 0;
	//pinMhlConn = 0;
	//pinUsbConn = 1;

	mhl_plug_unplug(1);
	//
	// Discovery over-ride: reg_disc_ovride	
	//
	SiiRegWrite(REG_MHLTX_CTL1, 0x10);

	fwPowerState = POWER_STATE_D0_MHL;

	//
	// Increase DDC translation layer timer (uint8_t mode)
	// Setting DDC Byte Mode
	//
	SiiRegWrite(REG_CBUS_COMMON_CONFIG, 0xF2);

	// Enable segment pointer safety
	//SET_BIT(0x0C44, 1);

	// Un-force HPD (it was kept low, now propagate to source
	//CLR_BIT(REG_INT_CTRL, 4);

	// Enable TMDS
	//SiiMhlTxDrvTmdsControl( true );

	
	// Change TMDS termination to 50 ohm termination (default)
	// Bits 1:0 set to 00
	SiiRegWrite(TX_PAGE_2 | 0x0001, 0x00);

	// Keep the discovery enabled. Need RGND interrupt
	ENABLE_DISCOVERY

	// Wait T_SRC_RXSENSE_CHK ms to allow connection/disconnection to be stable (MHL 1.0 specs)
//	TX_DEBUG_PRINT (("[%d] Drv: Wait T_SRC_RXSENSE_CHK (%d ms) before checking RSEN\n",
//							(int) (HalTimerElapsed( ELAPSED_TIMER ) * MONITORING_PERIOD),
//							(int) T_SRC_RXSENSE_CHK) );

	//
	// Ignore RSEN interrupt for T_SRC_RXSENSE_CHK duration.
	// Get the timer started
	//
//	HalTimerSet(TIMER_TO_DO_RSEN_CHK, T_SRC_RXSENSE_CHK);

	// Notify upper layer of cable connection
	SiiMhlTxNotifyConnection(mhlConnected = true);
}
///////////////////////////////////////////////////////////////////////////
//
// MhlTxDrvProcessDisconnection
//
///////////////////////////////////////////////////////////////////////////
static void MhlTxDrvProcessDisconnection (void)
{
	TX_DEBUG_PRINT(("Drv: MhlTxDrvProcessDisconnection\n"));

mhl_plug_unplug(0);
// clear all interrupts
	SiiRegWrite(REG_INTR4, SiiRegRead(REG_INTR4));

	SiiRegWrite(REG_MHLTX_CTL1, 0xD0);

    
	dsHpdStatus &= ~BIT6;  //cable disconnect implies downstream HPD low
	SiiRegWrite(REG_PRI_XFR_ABORT_REASON, dsHpdStatus);
	SiiMhlTxNotifyDsHpdChange(0);

	if( POWER_STATE_D0_MHL == fwPowerState )
	{
		// Notify upper layer of cable removal
		SiiMhlTxNotifyConnection(false);
	}

	// Now put chip in sleep mode
	SwitchToD3();
}

///////////////////////////////////////////////////////////////////////////
//
// CbusReset
//
///////////////////////////////////////////////////////////////////////////
void CbusReset (void)
{
uint8_t enable[4]={0xff,0xff,0xff,0xff};// must write 0xFF to clear regardless!
	SET_BIT(REG_SRST, 3);
	HalTimerWait(2);
	CLR_BIT(REG_SRST, 3);

	mscCmdInProgress = false;

	// Adjust interrupt mask everytime reset is performed.
    UNMASK_INTR_1_INTERRUPTS;
	UNMASK_INTR_4_INTERRUPTS;
    if (g_chipRevId < 1)
    {
    	if(fwPowerState != POWER_STATE_FIRST_INIT)
			UNMASK_INTR_5_INTERRUPTS;
	}
	else
	{
		//RG disabled due to auto FIFO reset
	    MASK_INTR_5_INTERRUPTS;
	}

	UNMASK_CBUS1_INTERRUPTS;
	UNMASK_CBUS2_INTERRUPTS;

		// Enable WRITE_STAT interrupt for writes to all 4 MSC Status registers.
    SiiRegWriteBlock(REG_CBUS_WRITE_STAT_ENABLE_0,enable,4);
		// Enable SET_INT interrupt for writes to all 4 MSC Interrupt registers.
    SiiRegWriteBlock(REG_CBUS_SET_INT_ENABLE_0,enable,4);
}

///////////////////////////////////////////////////////////////////////////
//
// CBusProcessErrors
//
//
///////////////////////////////////////////////////////////////////////////
static uint8_t CBusProcessErrors (uint8_t intStatus)
{
    uint8_t result          = 0;
    uint8_t mscAbortReason  = 0;
	uint8_t ddcAbortReason  = 0;

    /* At this point, we only need to look at the abort interrupts. */

    intStatus &= (BIT_CBUS_MSC_MR_ABRT| BIT_MSC_XFR_ABORT| BIT_DDC_ABORT);

    if ( intStatus )
    {
//      result = ERROR_CBUS_ABORT;		// No Retry will help

        /* If transfer abort or MSC abort, clear the abort reason register. */
		if( intStatus & BIT_DDC_ABORT)
		{
			result = ddcAbortReason = SiiRegRead(REG_DDC_ABORT_REASON);
			TX_DEBUG_PRINT(("CBUS DDC ABORT happened, reason:: %02X\n", (int)(ddcAbortReason)));
		}

        if ( intStatus & BIT_MSC_XFR_ABORT)
        {
            result = mscAbortReason = SiiRegRead(REG_PRI_XFR_ABORT_REASON);

            TX_DEBUG_PRINT(("CBUS:: MSC Transfer ABORTED. Clearing 0x0D\n"));
            SiiRegWrite(REG_PRI_XFR_ABORT_REASON, 0xFF);
			//MhlTxDrvProcessDisconnection();
        }
        if ( intStatus & BIT_CBUS_MSC_MR_ABRT)
        {
            TX_DEBUG_PRINT(("CBUS:: MSC Peer sent an ABORT. Clearing 0x0E\n"));
            SiiRegWrite(REG_CBUS_PRI_FWR_ABORT_REASON, 0xFF);
        }

        // Now display the abort reason.

        if ( mscAbortReason != 0 )
        {
            TX_DEBUG_PRINT(("CBUS:: Reason for ABORT is ....0x%02X = ", (int)mscAbortReason));

            if ( mscAbortReason & CBUSABORT_BIT_REQ_MAXFAIL)
            {
                TX_DEBUG_PRINT(("Requestor MAXFAIL - retry threshold exceeded\n"));
            }
            if ( mscAbortReason & CBUSABORT_BIT_PROTOCOL_ERROR)
            {
                TX_DEBUG_PRINT(("Protocol Error\n"));
            }
            if ( mscAbortReason & CBUSABORT_BIT_REQ_TIMEOUT)
            {
                TX_DEBUG_PRINT(("Requestor translation layer timeout\n"));
            }
            if ( mscAbortReason & CBUSABORT_BIT_PEER_ABORTED)
            {
                TX_DEBUG_PRINT( ("Peer sent an abort\n"));
            }
            if ( mscAbortReason & CBUSABORT_BIT_UNDEFINED_OPCODE)
            {
                TX_DEBUG_PRINT( ("Undefined opcode\n"));
            }
        }
    }
    return( result );
}

void SiiMhlTxDrvGetScratchPad (uint8_t startReg,uint8_t *pData,uint8_t length)
{

	SiiRegReadBlock(REG_CBUS_SCRATCHPAD_0+startReg, pData, length);
}

/*
 SiiMhlTxDrvMscMsgNacked
    returns:
        0 - message was not NAK'ed
        non-zero message was NAK'ed
 */
int SiiMhlTxDrvMscMsgNacked(void)
{
    if (SiiRegRead(REG_MSC_WRITE_BURST_LEN) & MSC_MT_DONE_NACK_MASK)
    {

        TX_DEBUG_PRINT(("MSC MSG NAK'ed - retrying...\n\n"));
        return 1;
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////
//
// MhlCbusIsr
//
// Only when MHL connection has been established. This is where we have the
// first looks on the CBUS incoming commands or returned data bytes for the
// previous outgoing command.
//
// It simply stores the event and allows application to pick up the event
// and respond at leisure.
//
// Look for interrupts on CBUS:CBUS_INTR_STATUS [0xC8:0x08]
//		7 = RSVD			(reserved)
//		6 = MSC_RESP_ABORT	(interested)
//		5 = MSC_REQ_ABORT	(interested)	
//		4 = MSC_REQ_DONE	(interested)
//		3 = MSC_MSG_RCVD	(interested)
//		2 = DDC_ABORT		(interested)
//		1 = RSVD			(reserved)
//		0 = rsvd			(reserved)
///////////////////////////////////////////////////////////////////////////
static void MhlCbusIsr (void)
{
	uint8_t		cbusInt;
	uint8_t     gotData[4];	// Max four status and int registers.

	//
	// Main CBUS interrupts on CBUS_INTR_STATUS
	//
	cbusInt = SiiRegRead(REG_CBUS_INTR_STATUS);

	// When I2C is inoperational (D3) and a previous interrupt brought us here, do nothing.
	if(cbusInt == 0xFF)
	{
		return;
	}
	
	if( cbusInt )
	{
		//
		// Clear all interrupts that were raised even if we did not process
		//
		SiiRegWrite(REG_CBUS_INTR_STATUS, cbusInt);

	    TX_DEBUG_PRINT(("Drv: Clear CBUS INTR_1: %02X\n", (int) cbusInt));
	}

	// MSC_MSG (RCP/RAP)
	if ((cbusInt & BIT_CBUS_MSC_MR_MSC_MSG))
	{
    	uint8_t mscMsg[2];
	    TX_DEBUG_PRINT(("Drv: MSC_MSG Received\n"));
		//
		// Two bytes arrive at registers 0x18 and 0x19
		//
        mscMsg[0] = SiiRegRead(REG_CBUS_PRI_VS_CMD);
        mscMsg[1] = SiiRegRead(REG_CBUS_PRI_VS_DATA);

	    TX_DEBUG_PRINT(("Drv: MSC MSG: %02X %02X\n", (int)mscMsg[0], (int)mscMsg[1] ));
		SiiMhlTxGotMhlMscMsg( mscMsg[0], mscMsg[1] );
	}
	if (cbusInt & (BIT_MSC_ABORT | BIT_MSC_XFR_ABORT | BIT_DDC_ABORT))
	{
		gotData[0] = CBusProcessErrors(cbusInt);
		mscCmdInProgress = false;
	}
	// MSC_REQ_DONE received.
	if (cbusInt & BIT_CBUS_MSC_MT_DONE)
	{
		TX_DEBUG_PRINT(("Drv: MSC_REQ_DONE\n"));

		mscCmdInProgress = false;

        // only do this after cBusInt interrupts are cleared above
		SiiMhlTxMscCommandDone( SiiRegRead(REG_CBUS_PRI_RD_DATA_1ST) );
	}
	//
	// Clear all interrupts that were raised even if we did not process
	//

	//
	// Now look for interrupts on register 0x1E. CBUS_MSC_INT2
	// 7:4 = Reserved
	//   3 = msc_mr_write_state = We got a WRITE_STAT
	//   2 = msc_mr_set_int. We got a SET_INT
	//   1 = reserved
	//   0 = msc_mr_write_burst. We received WRITE_BURST
	//
	cbusInt = SiiRegRead(REG_CBUS_MSC_INT2_STATUS);
	if(cbusInt)
	{
		//
		// Clear all interrupts that were raised even if we did not process
		//
		SiiRegWrite(REG_CBUS_MSC_INT2_STATUS, cbusInt);

	    TX_DEBUG_PRINT(("Drv: Clear CBUS INTR_2: %02X\n", (int) cbusInt));
	}

    if ( BIT_CBUS_MSC_MR_WRITE_BURST & cbusInt)
    {
        // WRITE_BURST complete
        SiiMhlTxMscWriteBurstDone( cbusInt );
    }

	if(cbusInt & BIT_CBUS_MSC_MR_SET_INT)
	{
    uint8_t intr[4];

	    TX_DEBUG_PRINT(("Drv: MHL INTR Received\n"));
	    SiiRegReadBlock(REG_CBUS_SET_INT_0, intr, 4);
	    SiiRegWriteBlock(REG_CBUS_SET_INT_0, intr, 4);
		// We are interested only in first two bytes.
		SiiMhlTxGotMhlIntr( intr[0], intr[1] );
	}

	if (cbusInt & BIT_CBUS_MSC_MR_WRITE_STATE)
	{
    uint8_t status[4];
    uint8_t clear[4]={0xff,0xff,0xff,0xff};// must write 0xFF to clear regardless!

		// don't put debug output here, it just creates confusion.
	    SiiRegReadBlock(REG_CBUS_WRITE_STAT_0, status, 4);
	    SiiRegWriteBlock(REG_CBUS_WRITE_STAT_0, clear, 4);
		SiiMhlTxGotMhlStatus( status[0], status[1] );
	}

}

static void ProcessScdtStatusChange(void)
{
	uint8_t scdtStatus;
	uint8_t mhlFifoStatus;

	scdtStatus = SiiRegRead(REG_TMDS_CSTAT);

	TX_DEBUG_PRINT(("Drv: ProcessScdtStatusChange scdtStatus: 0x%02x\n", scdtStatus));

	if (scdtStatus & 0x02)
	{
		 mhlFifoStatus = SiiRegRead(REG_INTR5);
		 TX_DEBUG_PRINT(("MHL FIFO status: 0x%02x\n", mhlFifoStatus));
		 if (mhlFifoStatus & 0x0C)
		 {
			SiiRegWrite(REG_INTR5, 0x0C);

			TX_DEBUG_PRINT(("** Apply MHL FIFO Reset\n"));
			SiiRegWrite(REG_SRST, 0x94);
			SiiRegWrite(REG_SRST, 0x84);
		 }
	}
}
/*
	SiMhlTxDrvSetClkMode
	-- Set the hardware this this clock mode.
 */
void SiMhlTxDrvSetClkMode(uint8_t clkMode)
{
	TX_DEBUG_PRINT(("SiMhlTxDrvSetClkMode:0x%02x\n",(int)clkMode));
	// nothing to do here since we only suport MHL_STATUS_CLK_MODE_NORMAL
	// if we supported SUPP_PPIXEL, this would be the place to write the register
}



uint16_t SiiMhlTxDrvGetDeviceId(void)
{
uint16_t retVal;
    retVal =  SiiRegRead(REG_DEV_IDH);
	retVal <<= 8;
	retVal |= SiiRegRead(REG_DEV_IDL);
    return retVal;
}
/*
    SiiMhlTxDrvGetDeviceRev
    returns chip revision
 */
uint8_t SiiMhlTxDrvGetDeviceRev(void)
{
    return SiiRegRead(REG_DEV_REV);
}


///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxDrvPowBitChange
//
// Alert the driver that the peer's POW bit has changed so that it can take 
// action if necessary.
//
void SiiMhlTxDrvPowBitChange (bool_t enable)
{
	// MHL peer device has it's own power
	if (enable)
	{
		SiiRegModify(REG_DISC_CTRL8, 0x04, 0x04);
	    TX_DEBUG_PRINT(("Drv: POW bit 0->1, set DISC_CTRL8[2] = 1\n"));
	}

#if (VBUS_POWER_CHK == ENABLE)
		if( vbusPowerState != enable)
		{
			vbusPowerState = enable;
			AppVbusControl( vbusPowerState );
		}
#endif
}

#if (VBUS_POWER_CHK == ENABLE)

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: MHLSinkOrDonglePowerStatusCheck()
//
// Function Description: Check MHL device is dongle or sink to set inputting current limitation.
//
void MHLSinkOrDonglePowerStatusCheck (void)
{
	uint8_t RegValue;

	if( POWER_STATE_D0_MHL == fwPowerState )
	{
		SiiRegWrite( REG_CBUS_PRI_ADDR_CMD, MHL_DEV_CATEGORY_OFFSET ); 	// DevCap 0x02
		SiiRegWrite( REG_CBUS_PRI_START, MSC_START_BIT_READ_REG ); // execute DevCap reg read command

		RegValue = SiiRegRead( REG_CBUS_PRI_RD_DATA_1ST );
		TX_DEBUG_PRINT(("[MHL]: Device Category register=0x%02X...\n", (int)RegValue));
	
		if( MHL_DEV_CAT_DONGLE == (RegValue & 0x0F) )
			{
				TX_DEBUG_PRINT(("[MHL]: DevTypeValue=0x%02X, the peer is a dongle, please limit the VBUS current input from dongle to be 100mA...\n", (int)RegValue));
			}
		else if( MHL_DEV_CAT_SINK == (RegValue & 0x0F) )
			{
				TX_DEBUG_PRINT(("[MHL]: DevTypeValue=0x%02X, the peer is a sink, limit the VBUS current input from sink to be 500mA...\n", (int)RegValue));
			}
	}
}
#endif

