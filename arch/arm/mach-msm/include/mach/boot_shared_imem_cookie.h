#ifndef BOOT_SHARED_IMEM_COOKIE_H
#define BOOT_SHARED_IMEM_COOKIE_H

/*===========================================================================

                                Boot Shared Imem Cookie
                                Header File

GENERAL DESCRIPTION
  This header file contains declarations and definitions for Boot's shared 
  imem cookies. Those cookies are shared between boot and other subsystems 

Copyright  2011, 2012 by QUALCOMM, Incorporated.  All Rights Reserved.
============================================================================*/

/*===========================================================================

                           EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/core/pkg/bootloaders/rel/1.0.1.9/boot_images/core/boot/secboot3/common/boot_shared_imem_cookie.h#2 $
  $DateTime: 2012/05/18 14:52:33 $ 
  $Author: poonams $

when       who          what, where, why
--------   --------     ------------------------------------------------------
03/20/12   kedara       Changed UEFI_CRASH_DUMP_MAGIC_NUM value to 0x1. 
06/07/11   kedara       Added L2 Dump Address.
06/07/11   dh           Initial creation
============================================================================*/

/*=============================================================================

                            INCLUDE FILES FOR MODULE

=============================================================================*/
#include <linux/types.h>
#include <linux/compiler-gcc.h>

/*=============================================================================

            LOCAL DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains local definitions for constants, macros, types,
variables and other items needed by this module.

=============================================================================*/

/* 
 * Following magic number indicates the boot shared imem region is initialized
 * and the content is valid
 */
#define BOOT_SHARED_IMEM_MAGIC_NUM        0xC1F8DB40

/* 
 * Magic number for UEFI ram dump
 */
#define UEFI_CRASH_DUMP_MAGIC_NUM         0x1

/* Default value used to initialize shared imem region */
#define SHARED_IMEM_REGION_DEF_VAL  0xFFFFFFFF 

/*
 * Type Definition
 */
typedef struct {
  uint32_t magic_1;
  uint32_t magic_2;
  uint32_t fusion_type;
  uint32_t fusion_status;
} __packed ftm_fusion_info;


/* 
 * Following structure defines all the cookies that have been placed 
 * in boot's shared imem space.
 * The size of this struct must NOT exceed SHARED_IMEM_BOOT_SIZE
 */
__packed struct boot_shared_imem_cookie_type
{
  /* First 8 bytes are two dload magic numbers */
  uint32_t dload_magic_1;
  uint32_t dload_magic_2;
  
  /* Magic number which indicates boot shared imem has been initialized
     and the content is valid.*/ 
  uint32_t shared_imem_magic;
  
  /* Magic number for UEFI ram dump, if this cookie is set along with dload magic numbers,
     we don't enter dload mode but continue to boot. This cookie should only be set by UEFI*/
  uint32_t uefi_ram_dump_magic;
  
  /* Pointer that points to etb ram dump buffer, should only be set by HLOS */
  uint32_t etb_buf_addr;
  
  /* Region where HLOS will write the l2 cache dump buffer start address */
  uint32_t *l2_cache_dump_buff_ptr;
  
  /* Please add new cookie here, do NOT modify or rearrange the existing cookies*/
  ftm_fusion_info fusion_info;

  /* ZTE_BOOT_20120823_SLF Pointer that points to hw reset flag address */
  uint32_t hw_reset_addr;

  /*
   * Added for EFS sync during system suspend & restore
   * by ZTE_BOOT_JIA_20120827 jia.jia
   */
  uint32_t app_suspend_state;
  uint32_t modemsleeptime; //sleeptime
  /* Please add new cookie here, do NOT modify or rearrange the existing cookies*/
};


/*  Global pointer points to the boot shared imem cookie structure  */
extern struct boot_shared_imem_cookie_type *boot_shared_imem_cookie_ptr;


#endif /* BOOT_SHARED_IMEM_COOKIE_H */

