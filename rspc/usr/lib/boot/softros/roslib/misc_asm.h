/* @(#)40	1.1  src/rspc/usr/lib/boot/softros/roslib/misc_asm.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:31  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************/
/* FILE NAME:    misc_asm.h                                                  */
/*               Public routine prototypes for misc_asm.s                    */
/*****************************************************************************/

#ifndef _H_MISC_ASM
#define _H_MISC_ASM


/*---------------------------------------------------------------------------*/
/*               Public function prototypes....                              */

extern void rtc_dec( uint delay );
extern void set_dec( uint value );
extern uint read_dec( void );
extern uint read_rtcu( void );
extern uint read_rtcl( void );
void        set_rtcu( uint );
void        set_rtcl( uint );
extern uint get_msr( void );
extern void set_msr( uint msr_value );
extern uint read_msr_ee( void );
extern void set_msr_ee( void );
extern void reset_msr_ee( void );
extern uint get_srr0( void );
extern uint get_srr1( void );
extern uint get_DAR( void );
extern uint get_DSISR( void );
extern uint get_PID( void );
extern uint get_PVR( void );
extern void set_seg_reg( uint seg_reg_num, uint seg_reg_value );
extern uint get_seg_reg( uint seg_reg_num );
extern void halt( void );

/*---------------------------------------------------------------------------*/
/*               Common equates....                                          */

#define  SEG_REG_0            0x0
#define  SEG_REG_1            0x1
#define  SEG_REG_2            0x2
#define  SEG_REG_3            0x3
#define  SEG_REG_4            0x4
#define  SEG_REG_5            0x5
#define  SEG_REG_6            0x6
#define  SEG_REG_7            0x7
#define  SEG_REG_8            0x8
#define  SEG_REG_9            0x9
#define  SEG_REG_A            0xA
#define  SEG_REG_B            0xB
#define  SEG_REG_C            0xC
#define  SEG_REG_D            0xD
#define  SEG_REG_E            0xE
#define  SEG_REG_F            0xF

#define  MSR_EE_BIT_MASK      0x8000    /* External interrupt bit in MSR */

#define EXT_INTS_ON    0x1              /* Return values of read_msr_ee  */
#define EXT_INTS_OFF   0x0              /* routine.                      */

#endif  /* _H_MISC_ASM */
