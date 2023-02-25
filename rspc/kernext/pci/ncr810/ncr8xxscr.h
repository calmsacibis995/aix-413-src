/* @(#)47	1.1  src/rspc/kernext/pci/ncr810/ncr8xxscr.h, pciscsi, rspc41J, 9507A 1/25/95 14:12:01 */
/*
 *   COMPONENT_NAME: PCISCSI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_NCR8XXSCR
#define _H_NCR8XXSCR


/************************************************************************/
/*									*/
/* SCRIPTS stuff							*/
/*									*/
/************************************************************************/

/*
 * Defines for our scripts
 */

#define P8XX_NOP_PATCH		0x80000000
#define P8XX_INTR_PATCH		0x98080000
#define P8XX_JUMP_PATCH		0x80080000
#define P8XX_MOVE1_PATCH	0x0E000001
#define P8XX_MOVE2_PATCH	0x0E000002
#define P8XX_MOVE3_PATCH	0x0E000003
#define P8XX_MOVE4_PATCH	0x0E000004
#define P8XX_MOVE5_PATCH	0x0E000005
#define P8XX_MOVE6_PATCH	0x0E000006

/*
 * Additional defines used in patching, already byte-swapped
 */

#define P8XX_BLOCK_RECEIVE		0x00000019
#define P8XX_CHAINED_RECEIVE		0x00000011
#define P8XX_RETURN_NOT_DATA_OUT	0x00000390
#define P8XX_RETURN_NOT_DATA_IN		0x00000391
#define P8XX_JUMP_WHEN_NOT_DATA_OUT	0x00000380
#define P8XX_JUMP_WHEN_NOT_DATA_IN	0x00000381
#define P8XX_MOVE_SCRATCHA2_AND_09	0x00093674



/*
 * These are interrupt codes that are patched into the SCRIPTS.
 * Watch out for conflicts with those defined in ncr8xx.ss!
 */
#define A_bdr_select_failed	0x0000001B
#define A_bdr_io_complete	0x0000001C
#define A_bdr_msg_error		0x0000001F
#define A_target_sdtr_sent	0x00000028
#define A_target_wdtr_sent	0x00000029
#define A_unknown_ext_msg	0x0000002A
#define A_modify_data_ptr	0x0000002B

/* 
 * Were including part of the NCR NASM compiler output here.  Other
 * parts are included by the ncr8xx*.c files.
 */
#include "ncr8xxss.h"		/* include the SCRIPTS compiler output */

#endif	/* _NCR8XXSCR */
