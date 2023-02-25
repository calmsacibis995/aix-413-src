/* @(#)90	1.6  src/tenplus/helpers/dir/index.h, tenplus, tenplus411, GOLD410 3/23/93 11:55:58 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
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
* File: index.h: include file for the index helper.                          *
*****************************************************************************/

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <string.h>
#include <DT.h>
#include "edexterns.h"
#include <keynames.h>
#include "INindex_msg.h"
#include "variables.h"
#include "statmod.h"


/* Names of files maintained by the index helper */

#define INDEX_NAME        ".index"
#define PUTDIR_NAME       ".putdir"


/* Display states */

#define ALL_FILES      (0)
#define VISIBLE_FILES  (1)


/* Form being viewed */

#define UNKNOWN_FORM   (-1)
#define INDEX_FORM     (0)
#define LS_FORM        (1)
#define STAT_FORM      (2)

/* Macros for referring to fields and flags values in the data file ... */
#include "fieldnames.h"



/* Miscellaneous defines */

#ifndef LOCAL
#define LOCAL           static
#endif
#define ROOT            (0)
#ifndef major
#define major(x)        (int) (((unsigned) (x) >> 16) & 0x7FFF)
#endif
#ifndef minor
#define minor(x)        (int) ((x) & 0xFFFF)
#endif
#define LONG_FORM       (1)
#define SHORT_FORM      (0)

/* Data types used in constructing the index file */

typedef char     *ASTRING;      /* Allocated string                       */
typedef ASTRING  *C_RECORD;     /* Composite: NAME, TYPE, and DESCRIPTION */
typedef C_RECORD *RECORD_LIST;  /* Set of C_RECORDs                       */

/* This is the number of items in the Usavestate tree */
#define NUMTOSAVE 6

/* Global variables */

extern ASTRING      Current_directory;
extern int          Display_state;
extern int          Form_type;
extern ASTRING      Index_file_name;
extern char         Last_stat_file[];
extern int          Pd_exists;
extern ASTRING      Put_directory;
extern int          Result_of_stat;
extern struct stat  Stat_buffer;
extern int          Writable_directory;


/* Function declarations */

extern void     add_ls_data(void);
extern POINTER *add_stat_data(POINTER *, int);
extern int      check_bits(char *, int);
extern char    *strdate(time_t);
extern int      uprepare(int, ASTRING, int, int);
extern void     urestore(int);
extern int      usave(int , ASTRING, int, int);
extern void     get_index_file(void);
