/* @(#)42	1.1  src/rspc/usr/lib/boot/softros/roslib/misc_sub.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:35  */
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
/* FILE NAME:    misc_sub.h                                                  */
/*               Public routine prototypes for misc_sub.c                    */
/*****************************************************************************/

#ifndef _H_MISC_SUB
#define _H_MISC_SUB
/*---------------------------------------------------------------------------*/
/*               Public structures....                                       */

typedef struct { unsigned int rtc_upper,
                              rtc_lower;
                                          }  rtc_value;


typedef struct { unsigned int upper_word,
                              lower_word;
                                          }  double_uint;

/*---------------------------------------------------------------------------*/
/*               Public function prototypes....                              */

void turn_ext_ints_off(void);
void turn_ext_ints_on(void);


double_uint div_double_by_short(double_uint number1, short number2, 
                                            unsigned int *remainder);
double_uint mult_double_by_short(double_uint number1, short number2, 
                                               unsigned int *overflow);


rtc_value div_rtc_by_short(rtc_value number1, short number2, 
                                            unsigned int *remainder);
rtc_value mult_rtc_by_short(rtc_value number1, short number2, 
                                               unsigned int *overflow);

rtc_value convert_to_rtc(unsigned int delta_time, int units);
rtc_value convert_from_rtc( rtc_value);
void get_rtc(rtc_value *return_value);
rtc_value    get_timeout_value( unsigned int delta_time, int units);
rtc_value    time_til_timeout( rtc_value rtc_timeout_value);
unsigned int timeout( rtc_value rtc_timeout_value );

/*---------------------------------------------------------------------------*/
/*               Variable definitions, common equates.....                   */

#define NANOSECONDS     -1      /* 1 ns = 1 ns (sign indicates ns unit)      */
#define MICROSECONDS  -1000     /* 1 us = 1000 ns (sign indicates ns unit)   */
#define MILLISECONDS -1000000   /* 1 ms = 1000000 ns (sign indicates ns unit)*/
#define SECONDS          1      /* 1 second = 1 second                  */
#define MINUTES         60      /* 60 seconds in a minute               */
#define HOURS         3600      /* 60 * 60 seconds in an hour           */
#define DAYS         86400      /* 60 * 60 * 24 seconds in a day        */

#define NANOSEC_TO_SEC_CONVERSION  1000000000   /* 1 billion nanoseconds in a second */

#endif  /* _H_MISC_SUB */
