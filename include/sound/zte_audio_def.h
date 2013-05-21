/* Copyright (c) 2010-2012, ZTE Corp. All rights reserved.
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
#ifndef __ZTE_AUDIO_DEF__
#define __ZTE_AUDIO_DEF__

/* ZTE_Audio_CJ_120530, chenjun, 2012-05-30, start */
#if defined(CONFIG_MACH_JARVIS)
#define CONFIG_US_EURO_SWITCH 1
#endif

#define US_EURO_SWITCH_ON_MS 200
#define US_EURO_SWITCH_OFF_MS 15

extern void US_EURO_Switch(int enable); // chenjun
/* ZTE_Audio_CJ_120530, chenjun, 2012-05-30, end */


/* ZTE_Audio_CJ_120530, chenjun, 2012-05-30, start */
#if defined(CONFIG_MACH_JARVIS)
/*
PM_GPIO_18  PM_GPIO_19  MODE
H                  H                 NON-CLIP A
H                  L                  NON-CLIP B
L                  H                  NON-CLIP OFF
L                  L                   POWER DOWN
*/
#define CONFIG_USE_YD_AMP 1
#endif
/* ZTE_Audio_CJ_120530, chenjun, 2012-05-30, end */


/* ZTE_Audio_CJ_120530, chenjun, 2012-05-30, start */
#if defined(CONFIG_MACH_ELDEN) ||defined(CONFIG_MACH_GORDON) ||defined(CONFIG_MACH_HAYES) || defined(CONFIG_MACH_KISKA)
#define CONFIG_USE_SPK_BOOST 1
#endif
/* ZTE_Audio_CJ_120530, chenjun, 2012-05-30, end */


#endif /* __ZTE_AUDIO_DEF__ */
