/*NOTUSED*/
/* @(#)89	1.9  src/tenplus/include/editorprf.h, tenplus, tenplus411, GOLD410 3/23/93 12:01:52 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/*****************************************************************************
* File: editorprf.h - contains defines for the record numbers in editorprf.  *
*****************************************************************************/

/* Name of user's personal profile directory and file */
#define USER_PROFILE_DIR	"$HOME/profiles"
#define USER_PROFILE		USER_PROFILE_DIR"/editorprf" 

#ifndef SYSTEM_DIR
#define SYSTEM_DIR     "/usr/lib/INed"            /* Installation directory  */
#endif

#define SYS_PROFILE_DIR		SYSTEM_DIR
#define SYS_PROFILE		SYS_PROFILE_DIR"/editorprf"

/* field names for color */
#define NNORMAL "Nn"
#define NBACKGROUND "Nb"
#define NUNDERLINE "Nu"
#define NUNDERBKG "Nub"

/* mask for converting values found in color fields */
#define ALPHAMASK 0x40

/*
Prototypes of all functions defined in helpers/editorprf
*/

extern int N_ubefore (register int , int , char *, int );
extern int N_uinit (char *);
extern int N_umod (char *, int , char *);
extern int getcolor (char *);

extern int Hinit (char *);
extern int Hmod (char *, int , char *, char *);
extern void Hafter (int );
extern int Hbefore (int , int , char * , int);
extern void Hdel (char *, int , int );
extern void Hins (char *, int , int );

extern void N_uafter (int );
extern void N_udel (char *, int);
extern void N_uins (char *, int , int );
