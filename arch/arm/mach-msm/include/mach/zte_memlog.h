/* 
 * ========================================================================
 *  Copyright (c) 2012-2013 by ZTE Corporation.  All Rights Reserved.        
 *
 * ------------------------------------------------------------------------
 *  Modify History
 * ------------------------------------------------------------------------
 * When        Who          What 
 * ===========================================================================
 */

#ifndef ZTE_MEMLOG_H
#define ZTE_MEMLOG_H

#define ZTE_SMEM_RAM_PHYS        0x88C00000
#define ZTE_SMEM_LOG_INFO_BASE   ZTE_SMEM_RAM_PHYS
#define ZTE_SMEM_LOG_GLOBAL_BASE (ZTE_SMEM_RAM_PHYS + PAGE_SIZE)

typedef struct {
  uint32 app_suspend_state; // 0:run 1:suspend
} zte_smem_global;
#endif /* ZTE_MEMLOG_H */


