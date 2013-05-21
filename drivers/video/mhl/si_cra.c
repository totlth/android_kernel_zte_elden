//***************************************************************************
//!file     si_cra.c
//!brief    Silicon Image Device register I/O support.
//
// No part of this work may be reproduced, modified, distributed, 
// transmitted, transcribed, or translated into any language or computer 
// format, in any form or by any means without written permission of 
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "MHL_SiI8334.h"
#include "si_cra.h"
#include <linux/i2c.h>

struct i2c_client *sii8334_PAGE_TPI = NULL;
struct i2c_client *sii8334_PAGE_TX_L0 = NULL;
struct i2c_client *sii8334_PAGE_TX_L1 = NULL;
struct i2c_client *sii8334_PAGE_TX_2  = NULL;
struct i2c_client *sii8334_PAGE_TX_3  = NULL;
struct i2c_client *sii8334_PAGE_CBUS  = NULL;




static uint8_t        l_pageInstance[SII_CRA_DEVICE_PAGE_COUNT] = {0};

// Index to this array is the virtual page number in the MSB of the REG_xxx address values
// Indexed with siiRegPageIndex_t value shifted right 8 bits
// DEV_PAGE values must correspond to the order specified in the siiRegPageIndex_t array

pageConfig_t	g_addrDescriptor[SII_CRA_MAX_DEVICE_INSTANCES][SII_CRA_DEVICE_PAGE_COUNT] =
{
	{
	{ DEV_I2C_0,  DEV_PAGE_TPI_0	},	// TPI 0
	{ DEV_I2C_0,  DEV_PAGE_TX_L0_0	},	// TX 0 Legacy 0
	{ DEV_I2C_0,  DEV_PAGE_TX_L1_0	},	// TX 0 Legacy 1
	{ DEV_I2C_0,  DEV_PAGE_TX_2_0	},	// TX 0 Page 2 (not legacy)
	{ DEV_I2C_0,  DEV_PAGE_TX_3_0	},	// TX 0 Page 3 (not legacy)

	{ DEV_I2C_0,  DEV_PAGE_CBUS 	},	  // CBUS 0

	{ DEV_DDC_0,  DEV_PAGE_DDC_EDID },	// TX EDID DDC 0
	{ DEV_DDC_0,  DEV_PAGE_DDC_SEGM }	// TX EDID DDC 0 Segment address
	}
};



//------------------------------------------------------------------------------
// Function:    SiiRegRead
// Description: Read a one byte register.
//              The register address parameter is translated into an I2C slave
//              address and offset. The I2C slave address and offset are used
//              to perform an I2C read operation.
//------------------------------------------------------------------------------

uint8_t SiiRegRead ( SiiReg_t virtualAddr )
{
    uint8_t             value = 0;
    uint8_t             regOffset = (uint8_t)virtualAddr;
    pageConfig_t        *pPage;


    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];
	switch(pPage->address)
		{
		case DEV_PAGE_TPI_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x72!\n",pPage->address));
			value = i2c_smbus_read_byte_data(sii8334_PAGE_TPI, regOffset);
			break;
			/*
		case DEV_PAGE_TX_L0_0:	
			value = i2c_smbus_read_byte_data(sii8334_PAGE_TX_L0, regOffset);
			break;
			*/
		case DEV_PAGE_TX_L1_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x7A!\n",pPage->address));
			value = i2c_smbus_read_byte_data(sii8334_PAGE_TX_L1, regOffset);
			break;
		case DEV_PAGE_TX_2_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x92!\n",pPage->address));
			value = i2c_smbus_read_byte_data(sii8334_PAGE_TX_2, regOffset);
			break;
		case DEV_PAGE_TX_3_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x9A!\n",pPage->address));
			value = i2c_smbus_read_byte_data(sii8334_PAGE_TX_3, regOffset);
			break;
		case DEV_PAGE_CBUS:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0xc8!\n",pPage->address));
			value = i2c_smbus_read_byte_data(sii8334_PAGE_CBUS, regOffset);		
			break;
		default:
			TX_DEBUG_PRINT(("pPage->address:%02x is wrong!\n",pPage->address));
			break;
		}
    return( value );
}


//------------------------------------------------------------------------------
// Function:    SiiRegWrite
// Description: Write a one byte register.
//              The register address parameter is translated into an I2C slave
//              address and offset. The I2C slave address and offset are used
//              to perform an I2C write operation.
//------------------------------------------------------------------------------

void SiiRegWrite ( SiiReg_t virtualAddr, uint8_t value )
{
    uint8_t             regOffset = (uint8_t)virtualAddr;
    pageConfig_t        *pPage;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];
	
	switch(pPage->address)
		{
		case DEV_PAGE_TPI_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x72!\n",pPage->address));
			i2c_smbus_write_byte_data(sii8334_PAGE_TPI, regOffset, value);
			break;
			/*
		case DEV_PAGE_TX_L0_0:	
			i2c_smbus_write_byte_data(sii8334_PAGE_TX_L0, regOffset, value);
			break;
			*/
		case DEV_PAGE_TX_L1_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x7A!\n",pPage->address));
			i2c_smbus_write_byte_data(sii8334_PAGE_TX_L1, regOffset, value);
			break;
		case DEV_PAGE_TX_2_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x92!\n",pPage->address));
			i2c_smbus_write_byte_data(sii8334_PAGE_TX_2, regOffset, value);
			break;
		case DEV_PAGE_TX_3_0:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x9A!\n",pPage->address));
			i2c_smbus_write_byte_data(sii8334_PAGE_TX_3, regOffset, value);
			break;
		case DEV_PAGE_CBUS:
			TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0xc8!\n",pPage->address));
			i2c_smbus_write_byte_data(sii8334_PAGE_CBUS, regOffset, value);
			break;
		default:
			TX_DEBUG_PRINT(("pPage->address:%02x is wrong!\n",pPage->address));
			break;
		}
}

//------------------------------------------------------------------------------
// Function:    SiiRegModify
// Description: Reads the register, performs an AND function on the data using
//              the mask parameter, and an OR function on the data using the
//              value ANDed with the mask. The result is then written to the
//              device register specified in the regAddr parameter.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              mask    - Eight bit mask
//              value   - Eight bit data to be written, combined with mask.
// Returns:     None
//------------------------------------------------------------------------------

void SiiRegModify ( SiiReg_t virtualAddr, uint8_t mask, uint8_t value)
{
    uint8_t aByte;

    aByte = SiiRegRead( virtualAddr );
    aByte &= (~mask);                       // first clear all bits in mask
    aByte |= (mask & value);                // then set bits from value
    SiiRegWrite( virtualAddr, aByte );
}

//------------------------------------------------------------------------------
// Function:    SiiRegBitsSet
// Description: Reads the register, sets the passed bits, and writes the
//              result back to the register.  All other bits are left untouched
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              bits   - bit data to be written
// Returns:     None
//------------------------------------------------------------------------------

void SiiRegBitsSet ( SiiReg_t virtualAddr, uint8_t bitMask, bool_t setBits )
{
    uint8_t aByte;

    aByte = SiiRegRead( virtualAddr );
    aByte = (setBits) ? (aByte | bitMask) : (aByte & ~bitMask);
    SiiRegWrite( virtualAddr, aByte );
}

//------------------------------------------------------------------------------
// Function:    SiiRegBitsSetNew
// Description: Reads the register, sets or clears the specified bits, and
//              writes the result back to the register ONLY if it would change
//              the current register contents.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              bits   - bit data to be written
//              setBits- true == set, false == clear
// Returns:     None
//------------------------------------------------------------------------------

void SiiRegBitsSetNew ( SiiReg_t virtualAddr, uint8_t bitMask, bool_t setBits )
{
    uint8_t newByte, oldByte;

    oldByte = SiiRegRead( virtualAddr );
    newByte = (setBits) ? (oldByte | bitMask) : (oldByte & ~bitMask);
    if ( oldByte != newByte )
    {
        SiiRegWrite( virtualAddr, newByte );
    }
}
//------------------------------------------------------------------------------
// Function:    I2CReadBlock
// Description: Reads a block of data(can read big than 32 byte).
// Returns:     success flag
//
//------------------------------------------------------------------------------

static uint8_t I2CReadBlock( struct i2c_client *client, uint8_t RegAddr, uint8_t NBytes, uint8_t * Data )
{
	uint8_t ret;
	struct i2c_msg request[] = {
		{ .addr  = client->addr,
		  .len   = sizeof(RegAddr),
		  .buf   = &RegAddr, },
		{ .addr  = client->addr,
		  .flags = I2C_M_RD,
		  .len   = NBytes,
		  .buf   = Data, },
	};
		
	ret = i2c_transfer(client->adapter, request, ARRAY_SIZE(request));
	if (ret != ARRAY_SIZE(request)){
		TX_DEBUG_PRINT(("unable to read EDID block\n"));
		return -EIO;
		}
	return 0;
}

uint8_t    SiiRegReadBlock ( SiiReg_t virtualAddr, uint8_t *pBuffer, uint16_t count )
{
	uint8_t				ret=0;
	uint8_t			   regOffset = (uint8_t)virtualAddr;
	pageConfig_t 	   *pPage;


	virtualAddr >>= 8;
	pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];
	switch(pPage->address)
	   {
	   case DEV_PAGE_TPI_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x72!\n",pPage->address));
		   ret = I2CReadBlock(sii8334_PAGE_TPI, regOffset, count, pBuffer);
		   break;
	   case DEV_PAGE_TX_L1_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x7A!\n",pPage->address));
		   ret = I2CReadBlock(sii8334_PAGE_TX_L1, regOffset, count, pBuffer);
		   break;
	   case DEV_PAGE_TX_2_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x92!\n",pPage->address));
		   ret = I2CReadBlock(sii8334_PAGE_TX_2, regOffset,count, pBuffer);
		   break;
	   case DEV_PAGE_TX_3_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x9A!\n",pPage->address));
		   ret = I2CReadBlock(sii8334_PAGE_TX_3, regOffset,count, pBuffer);
		   break;
	   case DEV_PAGE_CBUS:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0xc8!\n",pPage->address));
		   ret = I2CReadBlock(sii8334_PAGE_CBUS, regOffset,count, pBuffer);	   
		   break;
	   default:
		   TX_DEBUG_PRINT(("pPage->address:%02x is wrong!\n",pPage->address));
		   TX_DEBUG_PRINT(("unable to read EDID block\n"));
		   ret =  -EIO;
		   break;
	   }
	return ret;

}

//------------------------------------------------------------------------------
// Function:    SiiRegEdidReadBlock
// Description: Reads a block of data from EDID record over DDC link.
// Parameters:  segmentAddr - EDID segment address (16 bit), including device page;
//              offsetAddr  - Sixteen bit register address, including device page.
// Returns:     success flag
//
//------------------------------------------------------------------------------

void SiiRegEdidReadBlock ( SiiReg_t segmentAddr, SiiReg_t virtualAddr, uint8_t *pBuffer, uint16_t count )
{
    uint8_t             regOffset = (uint8_t)virtualAddr;
    pageConfig_t        *pPage;


    // Read the actual EDID data
    regOffset = (uint8_t)virtualAddr;
    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];


	
	switch(pPage->address)
		{
		case DEV_PAGE_TPI_0:
			I2CReadBlock(sii8334_PAGE_TPI, (uint8_t)(regOffset), count, pBuffer);
			break;
			/*
		case DEV_PAGE_TX_L0_0:	
			I2CReadBlock(sii8334_PAGE_TX_L0, (uint8_t)(regOffset), count, pBuffer);
			break;
			*/
		case DEV_PAGE_TX_L1_0:
			I2CReadBlock(sii8334_PAGE_TX_L1, (uint8_t)(regOffset), count, pBuffer);
			break;
		case DEV_PAGE_TX_2_0:
			I2CReadBlock(sii8334_PAGE_TX_2, (uint8_t)(regOffset), count, pBuffer);
			break;
		case DEV_PAGE_TX_3_0:
			I2CReadBlock(sii8334_PAGE_TX_3, (uint8_t)(regOffset), count, pBuffer);
			break;
		case DEV_PAGE_CBUS:
			I2CReadBlock(sii8334_PAGE_CBUS, (uint8_t)(regOffset), count, pBuffer);
			break;
		default:
			TX_DEBUG_PRINT(("pPage->address:%02x is wrong!\n",pPage->address));
			break;
		}
/*
	{
		int	i;
		for (i=0; i<count; i++)
		{
			*pBuffer = I2C_ReadByte((uint8_t)pPage->address, (uint8_t)(regOffset + i));
			++pBuffer;
		}
	}
	*/

}


void    SiiRegWriteBlock ( SiiReg_t virtualAddr, const uint8_t *pBuffer, uint16_t count )
{
	uint8_t 		   regOffset = (uint8_t)virtualAddr;
	pageConfig_t	   *pPage;


	virtualAddr >>= 8;
	pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];
	switch(pPage->address)
	   {
	   case DEV_PAGE_TPI_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x72!\n",pPage->address));
		   i2c_smbus_write_i2c_block_data(sii8334_PAGE_TPI, regOffset, count, pBuffer);
		   break;
	   case DEV_PAGE_TX_L1_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x7A!\n",pPage->address));
		   i2c_smbus_write_i2c_block_data(sii8334_PAGE_TX_L1, regOffset, count, pBuffer);
		   break;
	   case DEV_PAGE_TX_2_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x92!\n",pPage->address));
		   i2c_smbus_write_i2c_block_data(sii8334_PAGE_TX_2, regOffset,count, pBuffer);
		   break;
	   case DEV_PAGE_TX_3_0:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0x9A!\n",pPage->address));
		   i2c_smbus_write_i2c_block_data(sii8334_PAGE_TX_3, regOffset,count, pBuffer);
		   break;
	   case DEV_PAGE_CBUS:
		   TX_I2C_DEBUG_PRINT(("pPage->address:%02x is 0xc8!\n",pPage->address));
		   i2c_smbus_write_i2c_block_data(sii8334_PAGE_CBUS, regOffset,count, pBuffer);    
		   break;
	   default:
		   TX_DEBUG_PRINT(("pPage->address:%02x is wrong!\n",pPage->address));
		   TX_DEBUG_PRINT(("unable to write EDID block\n"));
		   break;
	   }
}


