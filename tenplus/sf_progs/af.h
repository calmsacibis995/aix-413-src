/* @(#)07	1.6  src/tenplus/sf_progs/af.h, tenplus, tenplus411, GOLD410 1/12/94 17:56:10 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
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
* File: af.h - include file for attribute access functions.                  *
*****************************************************************************/

#include <IN/AFdefs.h>


typedef struct {
		   char *value;     /* Value attribute          */
		   char *name;      /* Name attribute, if any   */
		   char *raname;    /* Name of the record array */
    unsigned short int   count;     /* Count attribute          */
	     short int   type;      /* Type attribute           */
    unsigned       char  flags;     /* Flags attribute          */
} STANZA;


extern AFILE_t Afp;
extern STANZA  Stanza;


extern void free_stanza(void);
extern int  next_stanza(void);


#ifdef DEBUG
extern void attr_dump(ATTR_t);
extern void dump_stanza(void);
#endif
