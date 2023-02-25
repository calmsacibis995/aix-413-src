/*NOTUSED*/
/* @(#)53	1.3  src/tenplus/include/libshort.h, tenplus, tenplus411, GOLD410 7/18/91 12:45:50 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************
* libshort.h:  include file for short function types                          *
*****************************************************************************/

extern void  i_addtype(void);
extern short *i_cat(short *, short *);
extern short *i_delete(short *, int, int);
extern short *i_insert(short *, int, int);
extern void  sht_print(short *, int);
extern short *i_pad(short *, int);
extern int   sht_read(short *, FILE *);
extern short *i_realloc(short *, int);
extern int   i_seq(short *, short *);
extern void  i_smear(short, int, short *);
extern void  i_strcpy(short *, short *);
extern short *i_string(short *);
extern int   i_strlen(short *);
extern short *i_strncpy(short *, short *, int);
extern int   i_strncmp(short *, short *, int);
extern int   sht_write(short *, FILE *);
