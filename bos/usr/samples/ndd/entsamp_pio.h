/* "@(#)35  1.1  src/bos/usr/samples/ndd/entsamp_pio.h, entsamp, bos411, 9428A410j 1/14/94 13:38:23 */
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_pio.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
 */

#ifndef _H_ENTSAMP_PIO
#define _H_ENTSAMP_PIO

#include <sys/ioacc.h>

/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, then   */
/*  exception code is ORed into a variable called pio_rc.  This variable  */
/*  must declared in the routine that will be using these macros.         */
/*------------------------------------------------------------------------*/

enum pio_func
{
	GETC, GETS, GETSR, GETL, GETLR, PUTSTR, GETSTR,
	PUTC, PUTS, PUTSR, PUTL, PUTLR
};

#define PIO_RETRY_COUNT		3

#define ENT_GETPOS(addr, c) \
{ \
	uchar pos_c; \
	uint pos_i; \
	 	    \
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) { \
		BUS_GETCX((char *)(addr), (char *)(c)); \
		BUS_GETCX((char *)(addr), &pos_c); \
		if (*(char *)(c) == pos_c) \
			break; \
	} \
	if (pos_i >= PIO_RETRY_COUNT) \
		pio_rc = TRUE; \
}
	
#define ENT_PUTPOS(addr, c) \
{ \
	uchar pos_c; \
	uint pos_i; \
		    \
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) { \
		BUS_PUTCX((char *)(addr), (char)(c)); \
		BUS_GETCX((char *)(addr), &pos_c); \
		if ((char)(c) == pos_c) \
			break; \
	} \
	if (pos_i >= PIO_RETRY_COUNT) \
		pio_rc = TRUE; \
}
	
#define ENT_GETCX(addr, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETCX((char *)(addr), (char *)(c))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				GETC, (int)(addr), (long)(c), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETSX(addr, s) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETSX((short *)(addr), (short *)(s))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				GETS, (int)(addr), (long)(s), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETSRX(addr, sr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETSRX((short *)(addr), (short *)(sr))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				GETSR, (int)(addr), (long)(sr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETLX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETLX((long *)(addr), (long *)(lr))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				GETL, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETLRX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETLRX((long *)(addr), (long *)(lr))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				GETLR, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTCX(addr, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTCX((char *)(addr), (char)(c))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				PUTC, (int)(addr), (long)(c), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTSX(addr, sr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTSX((short *)(addr), (short)(sr))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				PUTS, (int)(addr), (long)(sr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTSRX(addr, sr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTSRX((short *)(addr), (short)(sr))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				PUTSR, (int)(addr), (long)(sr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTLX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTLX((long *)(addr), (long)(lr))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				PUTL, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTLRX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTLRX((long *)(addr), (long)(lr))) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				PUTLR, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTSTRX(addr, src, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTSTRX((char *)(addr), (char *)(src), (int)c)) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
				PUTSTR, (int)(addr), (long)(src), (int)c); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETSTRX(addr, src, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETSTRX((char *)(addr), (char *)(src), (int)c)) { \
		excpt_code = entsamp_pio_retry(p_dev_ctl, excpt_code, \
					   GETSTR, (int)(addr), (long)(src), \
					   (int) c); \
		pio_rc |= excpt_code; \
	} \
}

#endif /* _H_ENTSAMP_PIO */
