/* @(#)98	1.6  src/bos/usr/sbin/adfutil/global.h, cmdadf, bos411, 9428A410j 6/19/91 13:18:44 */
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS:  global.h - glabal definitions for adfutil
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

extern int yydebug;

char *sbFile = NULL;
int  cLines;
int  cErrors;

int iCardid;
int fquiet;
char *sbDevice = NULL;

int cItem = 1;
int cChoice;
int cPos;
int cRes;
int fPrtDebug;
int odmdebug = false;

struct fl *ucode = NULL;
struct fl *adf_ucode = NULL;
int fucode = 0;
char *sbAdap;

struct he *rgphe[iheMax];	/* hash table */
struct adft *proot;

nl_catd ADF_ERR;
