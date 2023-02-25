/* @(#)34  1.1  src/bos/usr/samples/ndd/entsamp_mac.h, entsamp, bos411, 9428A410j 1/14/94 13:38:10 */
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_mac.h
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

#ifndef _H_ENTSAMP_MAC
#define _H_ENTSAMP_MAC



#define ENTSAMP_MAX_ADAPTERS	16	/* max. adapters supported by driver */
#define ENTSAMP_DD_NAME		"entsamp"	/* driver name for dump */
#define ENTSAMP_TX_LIST		64	/* max. # of transmit buffers */
#define ENTSAMP_RV_LIST		64	/* max. # of receive buffers */

#define ENT_HDR_LEN		14	/* Ethernet header length */
#define MULTI_BIT_MASK          0x01    /* multicast bit in first addr. byte */
#define NUM_POS_REG     	8   	/* total number of POS registers */



/*
 * Internal trace table size and trace macros
 */

#define ENTSAMP_TRACE_END        0x45454545      /* 'EEEE' is end of trace */

#ifdef DEBUG
#define ENTSAMP_TRACE_SIZE	1024		/* 4096 bytes, 256 traces */

#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	entsamp_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	\
	entsamp_trace(hook, tag, arg1, arg2, arg3)
#else
#define ENTSAMP_TRACE_SIZE	128             /* 512 bytes, 32 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	TRCHKGT(hook | HKTY_GT | 4, tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	
#endif 

/*
 * Trace hook numbers.
 * These numbers should be unique on the system and they should be 
 * defined in the /usr/include/sys/cdli_entuser.h (for Ethernet drivers).
 * They are defined here just for the convenience.
 */

#define HKWD_ENTSAMP_XMIT        0x35100000      /* transmit events */
#define HKWD_ENTSAMP_RECV        0x35200000      /* receive events  */
#define HKWD_ENTSAMP_OTHER       0x35300000      /* other events    */
#define HKWD_ENTSAMP_ERR         0x35400000      /* error events    */

/* 
 * The component dump table will have the dd_ctl table,  and one dev_ctl 
 * for each adapter that is opened. Leave one extra entry always empty for
 * the table management.
 */
#define ENTSAMP_CDT_SIZE		(2 + ENTSAMP_MAX_ADAPTERS)


/* 
 * Lock class registration number for lock instrumentation. 
 * These numbers should be unique on the system and they should be 
 * controlled by the lock registration procedure set up for the lock 
 * instrumentations. They are defined here just for the convenience.
 */
#define ENTSAMP_DD_LOCK			100
#define ENTSAMP_TRACE_LOCK		101
#define ENTSAMP_SLIH_LOCK		102
#define ENTSAMP_TX_LOCK			103
#define ENTSAMP_CTL_LOCK		104
#define ENTSAMP_CMD_LOCK		105


/*
 * compare two ethernet network address
 */
#define SAME_NADR(a, b) ( \
        *((ulong *)(a)) == *((ulong *)(b)) && \
        *((ushort *)((char *)(a) + 4)) == *((ushort *)((char *)(b) + 4)) \
                        )

/*
 * delay mili-seconds
 */
#define DELAYMS(ms)             delay ((int)(((ms)*HZ+999)/1000))


#endif  /* _H_ENTSAMP_MAC */
