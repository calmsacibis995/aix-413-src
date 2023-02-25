static char sccsid[] = "@(#)26	1.3  src/rspc/usr/lib/boot/softros/roslib/misc_sub.c, rspc_softros, rspc41J, 9514A_all 3/31/95 17:00:02";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: div_double_by_short
 *		get_PVR
 *		if
 *		mult_double_by_short
 *		switch
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
#pragma pagesize(60)                          /* set listing page size   */
//#pragma linesize(132)                         /* set listing line size   */
#pragma title("misc_sub.c : Miscellaneous library routines")
#pragma subtitle("IBM Confidential")

#ifdef LISTON
#pragma options source list
#endif

//************************** OCO SOURCE MATERIALS *****************************
//******* IBM CONFIDENTIAL (IBM CONFIDENTIAL-RESTRICTED when combined *********
//************** with the aggregated modules for this product) ****************
//*****************************************************************************
// FILE NAME:          misc_sub.c
// FILE CREATED:       06/05/93 by D. Cronk
// FILE OWNER:
//
// COPYRIGHT:          (C) Copyright IBM Corp. 1993, 1993
//
// DESCRIPTION: This file contains miscellaneous library routines.
//
//
// REVISION HISTORY:
// -----------------------Sandalfoot-----------------------------------------
// Date      Track#  Who  Description
// --------  ------  ---  ---------------------------------------------------
//
//*****************************************************************************


/*---------------------------------------------------------------------------*/
/*               Include files, external function prototypes...              */

#ifdef LISTON
#pragma options nosource
#endif

#include <sys/types.h>
#include <io_sub.h>
#include <misc_asm.h>
#include <misc_sub.h>
#include <stdlib.h>

#ifdef LISTON
#pragma options source
#endif

/*---------------------------------------------------------------------------*/
/*               Variable definitions, common equates.....                   */


#define NS_FACT1        20000      // These two values are used in converting
#define NS_FACT2        50000      // a number of nanoseconds to a number of
                                   // seconds and a number of nanoseconds.
                                   // Notice that NS_FACT1 * NS_FACT2 is
                                   // equal to NANOSEC_TO_SEC_CONVERSION.


#define uP_601     1                     // Processor ID of the 601
#define uP_603     3                     // Processor ID of the 603
#define uP_603PLUS 6                     // Processor ID of the 603+
#define uP_604     4                     // Processor ID of the 604
#define uP_604PLUS 9                     // Processor ID of the 604+

#ifdef DEBUG2
unsigned int debug_print = TRUE;
#endif

   /* The following variables are used by the timing routines.  They */
   /* are set to the processor specific timing values so that the    */
   /* same timing routines can be used by different processors.      */

unsigned int conv_speed,                // Global conversion speed for timing.
             freq_adjust,               // Global freqency adjuster for timeing.
             max_lower_val;             // Rollover value for lower timing register.
unsigned int processor_type = NULL;     // Processor type (1 = 601, 3 = 603)

/*---------------------------------------------------------------------------*/
/*               Internal function definitions...                            */

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: turn_ext_ints_off           Turn external interrupts off
//
// DESCRIPTION:  This routine will disable external interrupts in the MSR.
//
// CALLING EXAMPLE:   turn_ext_ints_off();
//
// INPUT:    None...
//
// OUTPUT:   No formal return parameters.  External interrupts are disabled.
//
//*****************************************************************************

void turn_ext_ints_off(void)
 {

  reset_msr_ee();

  return;
 }


#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: turn_ext_ints_on           Turn external interrupts on
//
// DESCRIPTION:  This routine will enable external interrupts in the MSR.
//
// CALLING EXAMPLE:   turn_ext_ints_on();
//
// INPUT:    None...
//
// OUTPUT:   No formal return parameters.  External interrupts are enabled.
//
//*****************************************************************************

void turn_ext_ints_on(void)
 {

  set_msr_ee();

  return;
 }

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: div_double_by_short        64 bit unsigned int division
//
// DESCRIPTION:  This routine will divide one 64 bit unsigned int by
//               another 16 bit unsigned int.
//
// CALLING EXAMPLE:   result = div_double_by_short(number1, number2, &remainder);
//
// INPUT:    The first double_uint value is the number to be divided.  The
//           second value (unsigned short integer) is the number to divide by.
//           The third parameter should be the address of a unsigned int
//           location for the remainder to be stored.
//
// OUTPUT:   The routine returns the integer division result.  Also, the
//           remainder is stored at the specified location.
//
//***************************************************************************** */
double_uint div_double_by_short(double_uint number1, short number2,
                                                   unsigned int *remainder)
 {

  unsigned short a, b, c, d;
  double_uint result;


   a = number1.upper_word >> 16;   /* Scale the number into 16 bit base units */
   b = number1.upper_word & 0xffff;
   c = number1.lower_word >> 16;
   d = number1.lower_word & 0xffff;

   /* Now divide each 16 bit value (starting with the uppermost) and keep  */
   /* rolling the remainder down to the next 16 bit value.                 */

   result.upper_word = (a / number2) << 16;
   *remainder = a % number2;

   result.upper_word = result.upper_word | (((*remainder << 16) + b) / number2);
   *remainder = ((*remainder << 16) + b) % number2;

   result.lower_word = (((*remainder << 16) + c) / number2) << 16;
   *remainder = ((*remainder << 16) + c) % number2;

   result.lower_word = result.lower_word | (((*remainder << 16) + d) / number2);

   /* We have our result of number1 / number 2.  Now get our final remainder. */

   *remainder = ((*remainder << 16) + d) % number2;

   return(result);
 }



#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: mult_double_by_short     64 bit unsigned int multiplication
//
// DESCRIPTION:  This routine will multiply one 64 bit unsigned int by
//               another 16 bit unsigned int.
//
// CALLING EXAMPLE:   result = mult_double_by_short(number1, number2, &overflow);
//
// INPUT:    The first parameter is a double_uint value to by multiplied by
//           the second parameter, an unsigned short integer.  The third
//           parameter should be the address of a unsigned int location
//           for the overflow to be stored.
//
// OUTPUT:   The routine returns the integer multiplication result.  Also, the
//           16 bit overflow value is stored at the specified location (0 if
//           no overflow).
//
//***************************************************************************** */
double_uint mult_double_by_short(double_uint number1, short number2,
                                                  unsigned int *overflow)
 {

  unsigned short a, b, c, d;
  double_uint result;


   a = number1.upper_word >> 16;   /* Scale the number into 16 bit base units   */
   b = number1.upper_word & 0xffff;
   c = number1.lower_word >> 16;
   d = number1.lower_word & 0xffff;

   /* Now multiply each 16 bit value (starting with the lowermost) and keep  */
   /* rolling the overflow portion up to the next 16 bit value.              */

  *overflow = d * number2;
  result.lower_word = *overflow & 0xffff;

  *overflow = (*overflow >> 16) + (c * number2);
  result.lower_word = result.lower_word | ((*overflow & 0xffff) << 16);

  *overflow = (*overflow >> 16) + (b * number2);
  result.upper_word = *overflow & 0xffff;

  *overflow = (*overflow >> 16) + (a * number2);
  result.upper_word = result.upper_word | ((*overflow & 0xffff) << 16);

   /* We have our result of number1 * number 2.  Now get our final overflow. */

   *overflow = *overflow >> 16;

  return(result);

 }

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: get_rtc                    Get RTC or TB register value
//
// DESCRIPTION:  This routine will return the value of the RTC (601) or
//               Time Base (603/604) register.
//
// CALLING EXAMPLE:   get_rtc(&my_rtc);
//
// INPUT:    Address of rtc_value structure to fill in with RTC or TB value.
//
// OUTPUT:   Input rtc_value structure has been set to RTC or TB value.
//
//*****************************************************************************

void get_rtc(rtc_value *return_value)
 {
  unsigned int tmp_val_upper, ee_value;


  ee_value = read_msr_ee();
  turn_ext_ints_off();

  do {
  (*return_value).rtc_upper = read_rtcu();
  (*return_value).rtc_lower = read_rtcl();
  tmp_val_upper = read_rtcu();

  } while ((*return_value).rtc_upper != tmp_val_upper ); /* enddo */

  if (ee_value == EXT_INTS_ON) {
    turn_ext_ints_on();
  } /* endif */

  return;
 }

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: add_lower                  Add 32 bit number to 64 bit int
//
// DESCRIPTION: This routine will add a 32 bit number to a 64 bit timing
//              register value.  The input max_val specifies the number at
//              which the lower register value rolls into the upper register
//              value (601 - rollover at 999999999, 603 - rollover at 0xFFFFFFFF).
//
// CALLING EXAMPLE:  new_rtc = add_lower(current_rtc, 0xFFFF0000, 0xFFFFFFFF);
//
// INPUT:   The first parameter is the rtc_value structure that will be added
//          to the second parameter (an unsigned int).  The third parameter
//          specifies the value at which the lower register value rolls into
//          the upper register value.
//
// OUTPUT:  The returned value is the sum of the input rtc_value structure
//          and the input 32 bit unsigned int.  The returned value has been
//          adjusted to ensure the lower register value is not above the
//          rollover value specified.
//
//
//***************************************************************************** */
rtc_value add_lower(rtc_value first_num, unsigned int second_lower, unsigned int max_val)
 {
   rtc_value result;

    if (first_num.rtc_lower > max_val) {
       /* If the input lower register value is above the rollover limit, */
       /* go ahead and adjust it.                                        */
       first_num.rtc_upper = first_num.rtc_upper + first_num.rtc_lower / (max_val + 1);
       first_num.rtc_lower = first_num.rtc_lower % (max_val + 1);
    } /* endif */

    if (second_lower > max_val) {
       /* If the second 32 bit lower register value is above the rollover   */
       /* limit, go ahead and adjust it.                                    */
       first_num.rtc_upper = first_num.rtc_upper + second_lower / (max_val + 1);
       second_lower = second_lower % (max_val + 1);
    } /* endif */

    /* Finally add the numbers, avoiding overflow if the two lower values */
    /* added together are bigger than the rolloever value.                */

    if (first_num.rtc_lower <= (max_val - second_lower)) {
       result.rtc_lower = first_num.rtc_lower + second_lower;
       result.rtc_upper = first_num.rtc_upper;
    } else {
       result.rtc_lower = first_num.rtc_lower - (max_val - second_lower) - 1;
       result.rtc_upper = first_num.rtc_upper + 1;
    } /* endif */

   return(result);
 }

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: rtc_add                        Add two 64 bit integers
//
// DESCRIPTION: This routine will add two 64 bit integers to each other and
//        return the result.  A rollover value for the lower 32 bits can be
//        specified.
//
// CALLING EXAMPLE:  new_rtc = rtc_add(current_rtc, delta_rtc, 999999999);
//
// INPUT:   The first parameter is the rtc_value structure that will be added
//          to the second rtc_value structure.  The third parameter specifies
//          the value at which the lower register value rolls into the upper
//          register value.
//
// OUTPUT:  The returned value is the sum of the two input rtc_value structures.
//          The returned rtc_value has been adjusted to ensure the lower
//          register value is not above the rollover value specified.
//
//***************************************************************************** */
rtc_value rtc_add(rtc_value first_num, rtc_value second_num, unsigned int max_val)
 {
   rtc_value result;

    if (first_num.rtc_lower > max_val) {
       /* If the first input lower register value is above the rollover  */
       /* limit, go ahead and adjust it.                                 */
       first_num.rtc_upper = first_num.rtc_upper + first_num.rtc_lower / (max_val + 1);
       first_num.rtc_lower = first_num.rtc_lower % (max_val + 1);
    } /* endif */

    if (second_num.rtc_lower > max_val) {
       /* If the second input lower register value is above the rollover  */
       /* limit, go ahead and adjust it.                                  */
       second_num.rtc_upper = second_num.rtc_upper + second_num.rtc_lower / (max_val + 1);
       second_num.rtc_lower = second_num.rtc_lower % (max_val + 1);
    } /* endif */


    /* Finally add the numbers, avoiding overflow if the two lower values */
    /* added together are bigger than the rolloever value.                */

    if (first_num.rtc_lower <= (max_val - second_num.rtc_lower)) {
       result.rtc_lower = first_num.rtc_lower + second_num.rtc_lower;
       result.rtc_upper = first_num.rtc_upper + second_num.rtc_upper;
    } else {
       result.rtc_lower = first_num.rtc_lower - (max_val - second_num.rtc_lower) - 1;
       result.rtc_upper = first_num.rtc_upper + second_num.rtc_upper + 1;
    } /* endif */

   return(result);
 }

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: subtract_lower      Subtract 32 bit number from 64 bit int
//
// DESCRIPTION: This routine will subtract a 32 bit number from a 64 bit timing
//              register value.  The input max_val specifies the number at
//              which the lower register value rolls into the upper register
//              value (601 - rollover at 999999999, 603 - rollover at 0xFFFFFFFF).
//
// CALLING EXAMPLE:  new_rtc = subtract_lower(current_rtc, 500, 0xFFFFFFFF);
//
// INPUT:   The first parameter is the rtc_value structure that will be
//          subtracted by the second parameter (an unsigned int).  The third
//          parameter specifies the value at which the lower register value
//          rolls into the upper register value.  This third parameter also
//          determines the borrow value from the upper register to the lower
//          register.
//
// OUTPUT:  The returned value is the result of the input rtc_value structure
//          subtracted by the input 32 bit unsigned int.  The returned value
//          has been adjusted to ensure the lower register value is not above
//          the rollover value specified.
//
//***************************************************************************** */
rtc_value subtract_lower(rtc_value first_num, unsigned int second_lower, unsigned int max_val)
 {
   rtc_value result;

    if (first_num.rtc_lower > max_val) {
       /* If the first input lower register value is above the rollover  */
       /* limit, go ahead and adjust it.                                 */
       first_num.rtc_upper = first_num.rtc_upper + first_num.rtc_lower / (max_val + 1);
       first_num.rtc_lower = first_num.rtc_lower % (max_val + 1);
    } /* endif */

    if (second_lower > max_val) {
       /* If the second 32 bit lower register value is above the rollover   */
       /* limit, go ahead and adjust it.                                    */
       first_num.rtc_upper = first_num.rtc_upper - second_lower / (max_val + 1);
       second_lower = second_lower % (max_val + 1);
    } /* endif */

    /* Finally perform the subtraction, borrowing from the upper value to  */
    /* the lower value if necessary.                                       */

    if (first_num.rtc_lower >= second_lower) {
       result.rtc_lower = first_num.rtc_lower - second_lower;
       result.rtc_upper = first_num.rtc_upper;
    } else {
       result.rtc_lower = first_num.rtc_lower + (max_val - second_lower) + 1;
       result.rtc_upper = first_num.rtc_upper - 1;
    } /* endif */

   return(result);
 }


#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: div_rtc_by_short        64 bit unsigned int division
//
// DESCRIPTION:  This routine will divide one 64 bit timing register value
//               by a 16 bit unsigned int.  Before using this routine,
//               the 64 bit timing register value should have been converted
//               into a total number of tics or nanoseconds (ie: the max
//               rollover value for the lower register = 0xFFFFFFFF).
//
// CALLING EXAMPLE:   result = div_rtc_by_short(number1, number2, &remainder);
//
// INPUT:    The first rtc_value structure is the number to be divided.  The
//           second unsigned short integer value is the number to divide by.
//           The third parameter should be the address of a unsigned int
//           location for the remainder to be stored.
//
// OUTPUT:   The routine returns the integer division result.  Also, the
//           remainder is stored at the input location.
//
//***************************************************************************** */
rtc_value div_rtc_by_short(rtc_value number1, short number2,
                                                      unsigned int *remainder)
 {

  double_uint result;


   result.upper_word = number1.rtc_upper;
   result.lower_word = number1.rtc_lower;

   result = div_double_by_short(result, number2, remainder);

   number1.rtc_upper = result.upper_word;
   number1.rtc_lower = result.lower_word;

   return(number1);

 }



#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: mult_rtc_by_short     64 bit unsigned int multiplication
//
// DESCRIPTION:  This routine will multiply one 64 bit unsigned int by
//               another 16 bit unsigned int.  Before using this routine,
//               the 64 bit timing register value should have been converted
//               into a total number of tics or nanoseconds (ie: the max
//               rollover value for the lower register = 0xFFFFFFFF).
//
// CALLING EXAMPLE:   result = mult_rtc_by_short(number1, number2, &overflow);
//
// INPUT:    The first parameter is a double_uint value to by multiplied by
//           the second paramter, an unsigned short integer.  The third
//           parameter should be the address of a unsigned int location
//           for the overflow to be stored.
//
// OUTPUT:   The routine returns the integer multiplication result.  Also, the
//           16 bit overflow value is stored at the input location ( = 0 if
//           no overflow).
//
//***************************************************************************** */
rtc_value mult_rtc_by_short(rtc_value number1, short number2,
                                                   unsigned int *overflow)
 {

  double_uint result;

   result.upper_word = number1.rtc_upper;
   result.lower_word = number1.rtc_lower;

   result = mult_double_by_short(result, number2, overflow);

   number1.rtc_upper = result.upper_word;
   number1.rtc_lower = result.lower_word;

   return(number1);

 }



#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: init_rtc_parms      Initialize timing subsystem constants.
//
// DESCRIPTION:  This routine will initialize the processor specific
//               timing parameters.  The various timing routines use
//               these parameters to calculate the conversion between
//               seconds and timing register tics.
//
// CALLING EXAMPLE:  init_rtc_parms();
//
// INPUT:   None...
//
// OUTPUT:  No formal output parameters. The conversion speed, frequency
//          adjustment and maximum lower register value have been set for the
//          processor type.
//
//***************************************************************************** */
void init_rtc_parms(void)
 {

   processor_type = get_PVR() >> 16;

   switch (processor_type) {
   case uP_601:

#ifdef RTC_STD_FREQ
      /* For the 601 processor system with an external 7.8125 Mhz RTC oscillator, */
      /* the conversion speed and frequency adjustment values are set to 1.       */
      /* With these values set, the lower RTC register will count the number      */
      /* of nanoseconds, the upper register will count the number of seconds.     */
      /* Refer to the 601 spec for more details...                                */
      conv_speed = 1;
      freq_adjust = 1;

#else
      /* For the 601 processor, the timing constants are set for the Sandalfoot   */
      /* RTC register frequency (processor bus speed * 2 / 125 Mhz).  Notice that */
      /* the RTC lower register acts like a 23 bit counter (see 601 spec for      */
      /* details), so the maximum lower register value is 1 billion - 1.          */
      conv_speed = ((get_bus_speed_info() >> 16) & 0xFF) * 2;
      freq_adjust = 125;
#endif

      max_lower_val = NANOSEC_TO_SEC_CONVERSION - 1;

      break;

   case uP_603:
   case uP_603PLUS:
   case uP_604:
   case uP_604PLUS:
      /* For the 603 processor, the timing constants are set for the Sandalfoot  */
      /* TimeBase register frequency (processor bus speed / 4000).  The TB lower */
      /* register is a "normal" 32 bit counter, so the maximum lower register    */
      /* value is 0xFFFFFFFF.                                                    */

      conv_speed = (get_bus_speed_info() >> 16) & 0xFF;
      freq_adjust = 4000;

      max_lower_val = 0xFFFFFFFF;

     break;
   } /* endswitch */

   return;
 }


#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: convert_tics_to_rtc Convert # ns value to s:ns value
//                                      OR convert # tics to gigatics:tics
//
// DESCRIPTION:  This routine converts a 64 bit number to two 32 bit numbers
//               (upper 32 bits = # of billions, lower 32 bits = remainder).
//               In other words, this routine can be used to convert a number
//               of nanoseconds to a number of seconds and the remainder of
//               nanoseconds.  Alternately, it can convert a number of tics
//               into a number of "gigatics" and the remainder of tics.
//
// CALLING EXAMPLE:  my_rtc = convert_tics_to_rtc(num_nanosecs);
//
// INPUT:   The input rtc value structure (ie : a 64 bit value) is used in
//          the conversion.
//
// OUTPUT:  The output rtc value structure is the converted input value.
//          The upper output portion is equal to the input value divided
//          by one billion.  The lower output portion is equal to the
//          remainder of the input value that was left over from the division
//          by one billion.
//
//***************************************************************************** */
rtc_value convert_tics_to_rtc(rtc_value input)
 {
  unsigned int a, b, c, d;
  unsigned int  ra, rb, rc, rd, sa,sb,sc,sd;
  unsigned int  temp1, temp2, rem1, rem2;
  rtc_value result;


     a = input.rtc_upper >> 16;               /* Scale the number into 16bit base units       */
     b = input.rtc_upper & 0xffff;
     c = input.rtc_lower >> 16;
     d = input.rtc_lower & 0xffff;

     ra    = a / NS_FACT1;
     temp1 = a % NS_FACT1;

     rb    = ((temp1 << 16) + b) / NS_FACT1;
     temp2 = ((temp1 << 16) + b) % NS_FACT1;

     rc    = ((temp2 << 16) + c) / NS_FACT1;
     temp1 = ((temp2 << 16) + c) % NS_FACT1;

     rd    = ((temp1 << 16) + d) / NS_FACT1;

     rem1  = ((temp1 << 16) + d) % NS_FACT1;        /* DEFINITELY need this result          */


     sa    = ra / NS_FACT2;
     temp1 = ra % NS_FACT2;

     sb    = ((temp1 << 16) + rb) / NS_FACT2;
     temp2 = ((temp1 << 16) + rb) % NS_FACT2;

     sc    = ((temp2 << 16) + rc) / NS_FACT2;
     temp1 = ((temp2 << 16) + rc) % NS_FACT2;

     sd    = ((temp1 << 16) + rd) / NS_FACT2;

     rem2  = ((temp1 << 16) + rd) % NS_FACT2;       /* This one too                         */

     result.rtc_lower = rem2 * NS_FACT1 + rem1;
     result.rtc_upper = sc<<16 | sd;

     return(result);
 }


#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: convert_rtc_to_tics Convert s:ns value to # ns
//                                      OR convert gigatics:tics to # tics
//
// DESCRIPTION:  This routine converts a number of seconds and nanoseconds
//               (upper 32 bits = # of seconds, lower 32 bits = nanoseconds)
//               to a number of nanoseconds (64 bit value).  Alternately,
//               this routine can convert a number of gigatics and tics to
//               a total number of tics.
//
// CALLING EXAMPLE:  my_nanos = convert_tics_to_rtc(my_rtc);
//
// INPUT:   The input rtc value structure is used in the conversion.
//
// OUTPUT:  The output rtc value structure is the converted input value.
//          The 64 bit output number is equal to the upper input portion
//          multiplied by 1 billion and then added to the lower input portion.
//
//***************************************************************************** */
rtc_value convert_rtc_to_tics(rtc_value input)
 {
  unsigned int  a, b, c, d;
  unsigned int  ra, rb, rc, rd, sa,sb,sc,sd;
  unsigned int  temp1, temp2;
  rtc_value result;

     sa = sb = 0;
     sc = input.rtc_upper >> 16;               /* Scale the number into 16bit base units */
     sd = input.rtc_upper & 0xffff;


     temp1 = sd * NS_FACT2;
     rd    = temp1 & 0xffff;

     temp2 = (temp1 >> 16) + (sc * NS_FACT2);
     rc    = temp2 & 0xffff;

     temp1 = (temp2 >> 16) + (sb * NS_FACT2);
     rb    = temp1 & 0xffff;

     temp2 = (temp1 >> 16) + (sa * NS_FACT2);
     ra    = temp2 & 0xffff;


     temp1 = rd * NS_FACT1  + (input.rtc_lower & 0xffff);
     d     = temp1 & 0xffff;

     temp2 = (temp1 >> 16) + (rc * NS_FACT1) + (input.rtc_lower >> 16);
     c     = temp2 & 0xffff;

     temp1 = (temp2 >> 16) + (rb * NS_FACT1);
     b     = temp1 & 0xffff;

     temp2 = (temp1 >> 16) + (ra * NS_FACT1);
     a     = temp2 & 0xffff;

     result.rtc_upper = a<<16 | b;
     result.rtc_lower = c<<16 | d;

     return(result);
 }


#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: convert_to_rtc      Convert time value to timing reg # tics
//
// DESCRIPTION:  This routine converts a time value to a number of timing
//               register tics.  The corresponding tics value will be adjusted
//               for the processor implementation frequency.
//
// CALLING EXAMPLE:  add_rtc_value = convert_to_rtc(5, SECONDS);
//
// INPUT:       The first parameter is the number of timing units.  The second
//              parameter is the timing unit (NANOSECONDS, MICROSECONDS,
//              MILLISECONDS, SECONDS, MINUTES, HOURS, DAYS).
//
// OUTPUT:      The output value is the time value converted to the corresponding
//              number of timing register tics.  This value can be added to the
//              current timing register value to obtain a timeout value.
//
//***************************************************************************** */
rtc_value convert_to_rtc(unsigned int delta_time, int units)
 {
  rtc_value delta_rtc;
  unsigned int tmp;


   delta_rtc.rtc_lower = 0;
   delta_rtc.rtc_upper = 0;


   /* The init_rtc_parms procedure will set our processor specific */
   /* timing constants.                                            */

   if (processor_type == NULL) {
      init_rtc_parms();
   } /* endif */


   /* We'll see if we should convert to a higher unit.  This is intended to */
   /* prevent overflow during later calculations.                           */

   if ((delta_time >= 1000000000) && (units == NANOSECONDS)) {
      delta_rtc.rtc_upper = delta_time / 1000000000;
      delta_time = delta_time % 1000000000;
   } /* endif */

   if ((delta_time >= 1000000) && (units == MICROSECONDS)) {
      delta_rtc.rtc_upper = delta_time / 1000000;
      delta_time = delta_time % 1000000;
   } /* endif */


   if ((delta_time >= 1000) && (units == MILLISECONDS)) {
      delta_rtc.rtc_upper = delta_time / 1000;
      delta_time = delta_time % 1000;
   } /* endif */

   delta_time  = delta_time * abs(units);



   if (units >= SECONDS) {
      /* We've already calculated the delta time * # seconds,  so now   */
      /* we just have to add it to the upper value.                     */

      delta_rtc.rtc_upper = delta_rtc.rtc_upper + delta_time;
      delta_time = 0;
   } else {
      /* We've already added the delta portion to the upper word.  Now we'll add  */
      /* the delta portion to the lower word (with carry to upper if necessary).  */

      delta_rtc = add_lower(delta_rtc, delta_time, max_lower_val);
   } /* endif */


   /* The value in delta_rtc is now equal to a number of seconds ( upper ) */
   /* and a number of nanoseconds (lower).                                 */

   /* First we adjust from seconds/nanoseconds to the total number of    */
   /* nanoseconds.  Then, to convert from nanoseconds to number of tics, */
   /* we multiply by the conversion speed and divice by the frequency    */
   /* adjuster.                                                          */

   delta_rtc = convert_rtc_to_tics(delta_rtc);

   delta_rtc = mult_rtc_by_short(delta_rtc, conv_speed, &tmp);
   delta_rtc = div_rtc_by_short(delta_rtc, freq_adjust, &tmp);


   if (processor_type == uP_601) {
      /* On 601 systems, the number of tics must be adjusted to a */
      /* upper 32 bit number and a lower 24 bit number.  This can */
      /* be thought of as a Gigatic (upper) : tic (lower) format. */
      delta_rtc = convert_tics_to_rtc(delta_rtc);
   } /* endif */

   if (tmp != 0) {
      /* If the remainder wasn't zero, go ahead and round up by 1. */
      /* This ensures that our delay is not shorter than the       */
      /* requested amount of time.                                 */
      delta_rtc = add_lower(delta_rtc, 1, max_lower_val);
   } /* endif */



   return(delta_rtc);
 }


#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: convert_from_rtc    Convert timing reg # tics value to s:ns
//
// DESCRIPTION:  This routine converts a number of timing register tics to a
//               number of seconds and nanoseconds.
//
// CALLING EXAMPLE:  time_value = convert_from_rtc(timing_reg_delta);
//
// INPUT:       The input rtc_value structure is interpreted as number of
//              timing register tics in a 64 bit format.
//
// OUTPUT:      The output rtc_value structure is the number of seconds and
//              nanoseconds that correspond to the timing register value.
//              The upper word in the structure contains the seconds value,
//              the lower word contains the remaining nanoseconds value.
//
//***************************************************************************** */
rtc_value convert_from_rtc(rtc_value input_rtc)
 {
  rtc_value return_val;
  unsigned int tmp;

   return_val = input_rtc;


   /* The init_rtc_parms procedure will set our processor specific */
   /* timing constants.                                            */

   if (processor_type == NULL) {
      init_rtc_parms();
   } /* endif */


   if (processor_type == uP_601) {

      /* On the 601, the input value is in a converted Gigatic:tic */
      /* format.  This needs to be converted to a number of tics.  */

      return_val = convert_rtc_to_tics(return_val);

   } /* endif */


   /* To convert from a number of tics to a number of nanoseconds, we need   */
   /* to multiply by the frequency adjuster, divide by the conversion speed. */

   return_val = mult_rtc_by_short(return_val, freq_adjust, &tmp);
   return_val = div_rtc_by_short(return_val, conv_speed, &tmp);

   /* Now adjust the value to upper word = seconds, lower word = nanoseconds */
   return_val = convert_tics_to_rtc(return_val);


   return(return_val);

 } /* convert from rtc */

rtc_value    get_timeout_value( unsigned int delta_time, int units)
 {
   rtc_value return_value, delta_rtc;

   get_rtc(&return_value);

#ifdef DEBUG2
   if (debug_print==TRUE)
     printf("\nStarting rtc upper = 0x%x, lower = 0x%x\n",
                                           return_value.rtc_upper, return_value.rtc_lower);
#endif

   delta_rtc = convert_to_rtc(delta_time, units);

#ifdef DEBUG2
   if (debug_print==TRUE)
     printf("Delta rtc upper = 0x%x, lower = 0x%x\n",
                                            delta_rtc.rtc_upper, delta_rtc.rtc_lower);
#endif

   return_value = rtc_add(return_value,  delta_rtc, max_lower_val);

#ifdef DEBUG2
   if (debug_print==TRUE)
     printf("Timeout value = 0x%x, lower = 0x%x\n",
                                           return_value.rtc_upper, return_value.rtc_lower);
#endif

   return(return_value);

 }

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: time_til_timeout    Determine s:ns until timeout reached
//
// DESCRIPTION:  This routine will determine how many seconds and nanoseconds
//               until a timeout value is reached in the timing register.
//
// CALLING EXAMPLE:  wait_time = time_til_timeout(my_timeout);
//
// INPUT:        The input is a timeout value of the processor timing register
//               (assumably calculated by get_timeout_value).
//
//
// OUTPUT:       The output rtc_value structure has the number of seconds
//               (upper portion) and nanoseconds (lower portion) until the
//               timeout value is reached.  If the timeout has already been
//               reached, the routine returns 0.
//
//***************************************************************************** */
rtc_value    time_til_timeout( rtc_value rtc_timeout_value)
 {
  unsigned int upper_diff,
               lower_diff;
  rtc_value current_rtc_value, return_val;


   return_val.rtc_upper = 0;
   return_val.rtc_lower = 0;

   if (processor_type == NULL) {
      init_rtc_parms();
   } /* endif */

   get_rtc(&current_rtc_value);

   upper_diff = rtc_timeout_value.rtc_upper - current_rtc_value.rtc_upper;
   lower_diff = rtc_timeout_value.rtc_lower - current_rtc_value.rtc_lower;


   if (rtc_timeout_value.rtc_upper <= current_rtc_value.rtc_upper) {

      if ((rtc_timeout_value.rtc_upper == current_rtc_value.rtc_upper) &&
          (rtc_timeout_value.rtc_lower > current_rtc_value.rtc_lower)     ) {

         return_val.rtc_lower = lower_diff;

      }


   } else {
        /* rtc_timeout_value.rtc_upper > current_rtc_value.rtc_upper   */
        /* Timeout not reached yet, calculate difference.  Notice that */
        /* the subtract_lower routine will handle a borrow from the    */
        /* upper word to the lower word if necessary.                  */

        return_val.rtc_upper = upper_diff;
        return_val.rtc_lower = rtc_timeout_value.rtc_lower;
        return_val = subtract_lower(return_val, current_rtc_value.rtc_lower, max_lower_val);

   } /* endif */

   /* Now that we have the "delta" rtc value, adjust it to a number */
   /* of seconds and nanoseconds.                                   */

   return_val = convert_from_rtc(return_val);

   return(return_val);

 }

#pragma page()
//*****************************************************************************
// SUBROUTINE NAME: timeout             Check if timeout value reached
//
// DESCRIPTION:  This routine checks if a timeout value has been reached
//               in the processor timing register.
//
// CALLING EXAMPLE:     while (timeout(my_timeout) == FALSE) { ... }
//
// INPUT:        The input is a timeout value of the processor timing register
//               (assumably calculated by get_timeout_value).
//
//
// OUTPUT:       This routine returns TRUE if the timeout value is less than
//               or equal to the current value of the processor timing
//               register.  If the timeout value is greater than the current
//               processor timing register value, the routine returns FALSE.
//
//***************************************************************************** */
unsigned int timeout( rtc_value rtc_timeout_value )
 {
  unsigned int rc = FALSE;
  rtc_value current_rtc_value;

   /* Get current timing register value. */

   get_rtc(&current_rtc_value);

   /* Since the return code is set for FALSE as a default, we'll only check the */
   /* conditions that will result in a TRUE return code.                        */

   if (current_rtc_value.rtc_upper > rtc_timeout_value.rtc_upper) {
      rc = TRUE;
   } else {
      if ((current_rtc_value.rtc_upper == rtc_timeout_value.rtc_upper) &&
          (current_rtc_value.rtc_lower >= rtc_timeout_value.rtc_lower)    ) {
            rc = TRUE;
      } /* endif */
   } /* endif */

  return(rc);
 }

