/* @(#)79	1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Knum.h, libKJI, bos411, 9428A410j 7/23/92 00:28:30	*/
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Knum.h
 *
 * DESCRIPTIVE NAME:  
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*
 *     INCLUDE NUMBER CHECH 
 *----------------------------------------------------------------------*/

#define     NUM_ONLY    0x10		/* InputString is Number Only   */
#define     NUM_INCL	0x20		/* InputString is IncludeNumData*/
#define	    NUM_MORA	0x00		/* Number Code is Mora Code	*/
#define	    NUM_YOMI	0x01		/* Number Code is YOMI Code	*/
#define	    NUM_CHAR	0x02		/* Number Code is Char Code	*/
#define     NUM_DATA    0x01            /* Number Data Cehck            */
#define     NUM_ELSE    0x02            /* Else Number Data Cehck       */ 

#define     ALP_ONLY    0x10		/* InputString is Alpha  Only   */
#define     ALP_INCL	0x20		/* InputString is IncludeAlpData*/
#define	    ALP_MORA	0x00		/* Alpha  Code is Mora Code	*/
#define	    ALP_YOMI	0x01		/* Alpha  Code is YOMI Code	*/
#define	    ALP_CHAR	0x02		/* Alpha  Code is Char Code	*/
#define     ALP_DATA    0x01            /* Alpha  Data Cehck            */
#define     ALP_ELSE    0x02            /* Else Alpha  Data Cehck       */ 

#define	    NUM_MORA_0  0xcf		/* Mora 0			*/
#define	    NUM_MORA_1  0xd0		/* Mora 1			*/
#define	    NUM_MORA_2  0xd1		/* Mora 2			*/
#define	    NUM_MORA_3  0xd2		/* Mora 3			*/
#define	    NUM_MORA_4  0xd3		/* Mora 4			*/
#define	    NUM_MORA_5  0xd4		/* Mora 5			*/
#define	    NUM_MORA_6  0xd5		/* Mora 6			*/
#define	    NUM_MORA_7  0xd6		/* Mora 7			*/
#define	    NUM_MORA_8  0xd7		/* Mora 8			*/
#define	    NUM_MORA_9  0xd8		/* Mora 9			*/

#define	    NUM_YOMI_0  0x75		/* Yomi 0			*/
#define	    NUM_YOMI_9  0x7e		/* Yomi 9			*/
#define	    NUM_CHAR_0  0x30		/* Char 0			*/
#define	    NUM_CHAR_9  0x39		/* Char 9			*/

#define     NUM_INIT	0x00		/* Mora Check Buffer Reset	*/
#define	    NUM_CHECK	0x01		/* Mora Check Buffer Check      */
#define	    NUM_SET	0x02		/* Mora Check Buffer Set        */

#define	    NUM_FLAG0	0x1d
#define	    NUM_FLAG1	0x01
#define	    NUM_HINSI	0x00
#define	    NUM_TYPE 	0x06
/*
#define	    NUM_FLAG0	0x00
#define	    NUM_FLAG1	0x00
#define	    NUM_HINSI	0x7f
#define	    NUM_TYPE 	0x00
 */

#define     MAX_MORA_INDEX	0x0C

#define	    NEXT_END		0x00
#define     NEXT_CONT		0x01
#define     NEXT_CONT_CHECK	0x02
#define     NEXT_CONT_MAX	0x03

#define	    KANJI_RM 	0xB0

short	    ext_sys_conv  = 0;
short	    ext_usr_conv  = 0;

/*----------------------------------------------------------------------*
 *      System Dicitionary Addition
 *----------------------------------------------------------------------*/
#define  M_OLD_NUM_0	0x30		/* Old Mora code 0		*/
#define  M_OLD_NUM_9	0x39		/* Old Mora code 9		*/
#define  M_OLD_ALP_A	0x41		/* Old Mora code A		*/
#define  M_OLD_ALP_Z	0x5A		/* Old Mora code Z		*/
#define  M_toNUMBER	0x45		/* Old Mora Number -> E.Mora 	*/
#define  M_toALPHA	0x9f		/* Old Mora Alpha  -> E.Mora    */
#define  M_E_NUMBER_0	0xcf		/* E.mora code 0		*/
#define  M_E_NUMBER_9	0xd8		/* E.mora code 9		*/
#define  M_E_ALPHA_a	0xb5		/* E.mora code a		*/
#define  M_E_ALPHA_z	0xce		/* E.mora code z		*/
#define  M_E_ALPHA_A	0xe0		/* E.mora code A		*/
#define  M_E_ALPHA_Z	0xf9		/* E.mora code Z		*/
#define  M_E_OFFSET 	0x2b		/* Conversion offset		*/
#define  M_E_SHIFT	0x80		/* Chack Shift key 0x1d		*/
#define  M_E_MASK 	0x7f		/* Chack Shift key mask 	*/

static	uschar	OLDtoEMORA[256] = {
/*      0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */

/*0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*1*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*2*/ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
/*3*/ 0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x10,0x11,0x12,0x13,0x14,0x15,
/*4*/ 0x16,0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,
/*5*/ 0xef,0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0x00,0x00,0x00,0x00,0x00,
/*6*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*7*/ 0x00,0x00,0x00,0x00,0x00,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x00,

/*8*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*9*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*a*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*b*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*c*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*d*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*e*/ 0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
/*f*/ 0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0x00,0x00,0x00,0x00,0x00,0x00
};

