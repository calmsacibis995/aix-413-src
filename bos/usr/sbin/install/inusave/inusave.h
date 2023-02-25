/* @(#)11	1.11  src/bos/usr/sbin/install/inusave/inusave.h, cmdinstl, bos411, 9428A410j 6/18/93 12:30:36 */

/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inusave.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifndef _H_INUSAVE
#define _H_INUSAVE

#include <stdio.h>
#include <string.h>
#include <inuerr.h>


#include <instlmsg.h>
#include <commonmsg.h>
#include <inudef.h>
#include <inu_toc.h>
#include <inulib.h>


/********************************************************
 **      G  E  N  E  R  I  C       M  A  C  R  O  S
 ********************************************************/


#define SV_NPARAMS			2   /* # parameters for inusave */
#define DIR_PERMS			S_IRUSR | S_IWUSR | S_IXUSR

#define SORTED_AL   		"_inu_sorted.al"
#define SORTED_ACF  		"_inu_sorted.acf"
#define TEMP_UL     		"_inu_temp.ul"
#define TEMP_ARL    		"_inu_temp.arl"


/********************************************************
 **            P  R  O  T  O  T  Y  P  E  S
 ********************************************************/

#ifndef _NO_PROTO
#define _NO_PROTO

int  save_files           (char *, char *, char *, char *);
int  save_arfile          (char *, char *, char *, char *, FILE *, int *);
int  write_ar_stanza_rec  (char *, char *, char *, FILE *, char *, int *,int *);
int  save_both_types 	  (char *, FILE *, FILE *, FILE *, FILE * );
int  save_regfile         (char *, char *, FILE *, int *);
int  init_ctr        	  (char *, char *, int *);
int  write_reg_stanza_rec (char *, char *, FILE *, int *, int *);
char *get_member          (char *);
void strip                (char *);
    
#else

int  save_files           ();
int  save_arfile          ();
int  write_ar_stanza_rec  ();
int  save_both_types      ();
int  save_regfile    	  ();
int  init_ctr       	  ();
int  write_reg_stanza_rec ();
char *get_member          ();
void strip                ();

#endif  /* _NO_PROTO */

extern	errno;

#endif	/* _H_INUSAVE */
