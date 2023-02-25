/* @(#)80	1.4  src/bos/usr/sbin/crash/debug.h, cmdcrash, bos411, 9428A410j 6/15/90 20:31:26 */
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */


/* debug.h	7.1 - 87/06/15 - 19:41:32 */

/* Generalized debug field; one per procedure                             */
/* Note that while dfdata is a word-aligned structure                     */
/* a debug field need not be word-aligned.                                */

struct dfdata {
	unsigned char df1;      /* always DFMAGIC                         */
	unsigned char code;     /* debug code field                       */
	unsigned char df2;      /* always DFMAGIC                         */
	unsigned char flags;    /* depends on code                        */
	union {
	    short _framesize;   /* D_NORMAL: size of this proc's frame    */
	    long  _bigframe;    /* D_BIGFRAME                             */
		  /* high order 4 bits = lowest fp register pair saved;   */
		  /* in range 0-5; use 6 if no fp registers are saved.    */
	} _data;
};
#define framesize	_data._framesize
#define bigframe	_data._bigframe

#define DFMAGIC         0xdf

/* debug codes */

#define D_ERROR         0       /* can't find debug data                  */
#define D_NORMAL        1       /* normal c-like debug data               */
#define D_NOFRAME       2       /* assembler routine w/no frame           */
#define D_START         3       /* debug data of "start" proc             */
#define D_SIGNAL        4       /* debug data of signal frame             */
#define D_NOARGS        5       /* assembler routine no args or frame     */
#define D_BIGFRAME      6       /* if fp regs saved or framesize > 0x7fff */

/*  When code is D_NORMAL or D_BIGFRAME                                   */
/*      flags = xxxx1234                                                  */
/*              xxxx      lowest register saved                           */
/*                  1234  on  if nth argument is in saved in a register   */
/*                        off if nth argument is on stack                 */
/*                                                                        */
/*  Arguments saved in registers are assigned sequentially down from 13.  */
/*  If flags = 10010101, r9-r15 are saved, r13 holds arg2, r12 holds arg4.*/
