/* @(#)90	1.4.1.1  src/bos/usr/sbin/adfutil/externs.h, cmdadf, bos411, 9428A410j 7/12/93 16:54:33 */
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS:  externs.h -- declared externs for .c support files
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

extern char *sbFile;
extern int  cLines;
extern int  cErrors;

extern int iCardid;
extern int fquiet;
extern char *sbDevice;
extern int ffile;

extern struct fl *ucode;
extern struct fl *adf_ucode;
extern int fucode;
extern char *sbAdap;

extern int cItem;
extern int cChoice;
extern int cPos;
extern int cRes;
extern int fPrtDebug;
extern int odmdebug;

extern struct he *rgphe[];	/* hash table */
extern struct adft *proot;

char *MakeString();
struct he *Lookup();
struct adft *makeadft();
struct cl *attachcl();
struct cl *makecl();
struct fl *attachfl();
struct fl *makefl();
struct fr *makefr();
struct ni *attachni();
struct ni *makeni();
struct nml *makenml();
struct pl *attachpl();
struct pl *makepl();
struct rgl *attachrgl();
struct rgl *makergl();
struct rl *attachrl();
struct rl *makerl();
struct sl *attachsl();
struct sl *makesl();
