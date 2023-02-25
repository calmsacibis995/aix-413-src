/* @(#)35       1.1  src/rspc/kernext/pm/pmmi/pmdump/pmdumpdebug.h, pwrsysx, rspc41J, 9510A 3/8/95 11:50:24 */
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: Power Management Kernel Extension
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PMDUMPDEBUG
#define _H_PMDUMPDEBUG

#ifdef PM_DEBUG
#define PMDEBUG_0(A)			printf(A);
#define PMDEBUG_1(A,B)			printf(A,B);
#define PMDEBUG_2(A,B,C)		printf(A,B,C);
#define PMDEBUG_3(A,B,C,D)		printf(A,B,C,D);
#define PMDEBUG_4(A,B,C,D,E)		printf(A,B,C,D,E);
#define PMDEBUG_5(A,B,C,D,E,F)		printf(A,B,C,D,E,F);
#define PMDEBUG_6(A,B,C,D,E,F,G)	printf(A,B,C,D,E,F,G);
#else /* PM_DEBUG */
#define PMDEBUG_0(A)
#define PMDEBUG_1(A,B)
#define PMDEBUG_2(A,B,C)
#define PMDEBUG_3(A,B,C,D)
#define PMDEBUG_4(A,B,C,D,E)
#define PMDEBUG_5(A,B,C,D,E,F)
#define PMDEBUG_6(A,B,C,D,E,F,G)
#endif /* PM_DEBUG */

#ifdef PM_HIBERNATION_DEBUG
#define DEBUG_0(A)			printf(A);
#define DEBUG_1(A,B)			printf(A,B);
#define DEBUG_2(A,B,C)			printf(A,B,C);
#define DEBUG_3(A,B,C,D)		printf(A,B,C,D);
#define DEBUG_4(A,B,C,D,E)		printf(A,B,C,D,E);
#define DEBUG_5(A,B,C,D,E,F)		printf(A,B,C,D,E,F);
#define DEBUG_6(A,B,C,D,E,F,G)		printf(A,B,C,D,E,F,G);
#else	/* PM_HIBERNATION_DEBUG */
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#endif /* PM_HIBERNATION_DEBUG */

#define	DEBUG_HEADER(hp)						\
{									\
	DEBUG_1("header.wakeup_area = 0x%x\n", (hp)->wakeup_area);	\
	DEBUG_1("header.wakeup_area_length = 0x%x\n",(hp)->wakeup_area_length);\
	DEBUG_1("header.wakeup_code_offset = 0x%x\n",(hp)->wakeup_code_offset);\
	DEBUG_1("header.wakeup_code_length = 0x%x\n",(hp)->wakeup_code_length);\
	DEBUG_1("header.config_data_offset = 0x%x\n",(hp)->config_data_offset);\
	DEBUG_1("header.config_data_length = 0x%x\n",(hp)->config_data_length);\
	DEBUG_1("header.restart = 0x%x\n", (hp)->restart);		\
	DEBUG_1("header.image_offset = 0x%x\n", (hp)->image_offset);	\
	DEBUG_1("header.image_length = 0x%x\n", (hp)->image_length);	\
	DEBUG_1("header.bitmap_offset = 0x%x\n", (hp)->bitmap_offset);	\
	DEBUG_1("header.bitmap_length = 0x%x\n", (hp)->bitmap_length);	\
}

#define	DEBUG_CONFIG_DATA(cp)						\
{									\
	DEBUG_1("config.memory_size = 0x%x\n", (cp)->memory_size);	\
	DEBUG_1("config.residual_sum = 0x%x\n", (cp)->residual_sum);	\
}

#endif /* _H_PMDUMPDEBUG */
