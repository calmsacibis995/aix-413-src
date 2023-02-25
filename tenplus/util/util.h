/* @(#)56	1.4  src/tenplus/util/util.h, tenplus, tenplus411, GOLD410 3/23/93 12:18:58 */

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

/*
This include file contains prototypes for all functions defined in utils.
*/
 
void unlock(char *);
void lock (char *);
unsigned char *convert (char *, char *, unsigned char );
unsigned char findcanon (char *);
void doinseqs (int);
void domultseq (POINTER *);
void initialise_inseqs(int);
char *get_termname(void);
void store_default(void);
void load_key(unsigned char, unsigned char *);
void load_custom_key(unsigned char, unsigned char *, char *);
int same_function(unsigned char *, POINTER *);
int seqcomp (POINTER *, POINTER *);
char *name_change(char *);
char *makename (char *);
