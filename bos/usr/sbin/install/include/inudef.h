/* @(#)44  1.7  src/bos/usr/sbin/install/include/inudef.h, cmdinstl, bos411, 9428A410j 4/28/94 17:34:45 */

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_INUDEF
#define _H_INUDEF

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
/* #include <stdarg.h> */
/* #include <inulib.h> */

#define MSGSTR(Set,Num,Str)  catgets(catd,Set,Num,Str)
/* #define CATOPEN()            catd = catopen(MF_CMDINSTL_E,0) */
/* #define CATCLOSE()           catclose(catd)                  */
/* #define MF_CMDINSTL_E        "cmdinstl_e.cat"                */

#ifndef BLANK
#define BLANK       ' '
#endif

#ifndef TRUE
#define TRUE       (1 == 1)
#endif

#ifndef FALSE
#define FALSE      !TRUE
#endif

#ifndef NIL
#define NIL(t)     ((t *)0)
#endif

#ifndef MASK
#define MASK(rc)      ((rc & 0x0000ffff) >> 8)
#endif

#ifndef NUL
#define NUL        '\0'
#endif

#ifndef MALLOC
#define MALLOC(t,n)   ((t *)malloc(sizeof(t)*n))
#endif

#define LF         '\012'
#define NEWLINE    '\n'

#define TAPE_BFF    1
#define SUCCESS     0
#define FAILURE     -1
#define TOC_FAILURE -2

#define SLASH      '/'
#define DOT        '.'
#define TAB        '\t'

/* added for 3.1 */

#define TMP_BUF_SZ               2048 /* size of generic buffers        */
#define MAX_LPP_FULLNAME_LEN      145 /* lpp.opt.opt.opt.opt + NULL     */
#define MAX_LPP_OPTIONNAME_LEN     29 /* each of above tokens + NULL    */
#define MAX_PRQ_SIZE             1024 /* length of prereq file in bytes */
#define MAX_LANG_LEN               32 /* length of prereq file in bytes */
#define LEVEL_LEN              15*2+1 /* 00.00.0000.0000 + NULL         */

/* added for 3.2 */

#define MAX_LEVEL_LEN              27 /* vv.rr.mmmm.ffff.ppppppp + NULL */
#define MIN_LEVEL_LEN              16 /* vv.rr.mmmm.ffff + NULL         */
#define MAX_UPDATES               100 /* max updates per bff            */
#define MAX_PARTS                   3 /* max num. or arg to installp -O */
#define MAX_COM_LEN              5000 /* max command length for system  */
#define MEM_BUF_SZ               1024 /* size of memory buffers         */
#define YES                         1
#define YES_STR                   "1"
#define NO                          0
#define NO_STR                    "0"
#define USER_FORMAT                 0
#define SMIT_FORMAT                 1



#define NFLAGSET                       '0'
#define INUSAVE                  "INUSAVE"
#define INUSAVEDIR            "INUSAVEDIR"
#define ODMDIR                    "ODMDIR"
#define UPDATE_LIST          "update.list"
#define ARCHIVE_LIST        "archive.list"
#define SYS_INST_UPDT "/usr/sys/inst_updt"
#define DFLT_VPD_OR    "/usr/lib/objrepos"
#define STRING_LEN            2*PATH_MAX+1
#define FALSE_C                (char)FALSE
#define TRUE_C                  (char)TRUE

/* Reserved filesystem size requirement indexes */
#define  PAGESPACE       0    /* Pagespace requirements */
#define  INSTWORK        1    /* Installp working space requirements */
#define  USRSAVESPACE    2    /* Usr package savespace */
#define  DEFSAVESPACE    2    /* Default savespace, used by inusave */
#define  ROOTSAVESPACE   3    /* Root package savespace */
#define  SHARESAVESPACE  4    /* Share package savespace */
#define  FIRST_REAL_FS   (SHARESAVESPACE-PAGESPACE+1)

/* added for the inu_ls functions */

#define APAR_INFO_LEN  68    /* Max length of apar description to print */
#define CON_LEN         8    /* Max length of the content column */
#define DESC_LEN       11    /* Max length of the word description */
#define INFO_LEN       60    /* Max length of the information in desc. field */
#define FULL_LEVEL_LEN 26    /* Max length of the level and its name */
#define MAX_LINE_LEN   80    /* Max length of any line to be printed out */
#define OPTION_LEN     27    /* Max length of an option name to be printed */
#define PLT_LEN         3    /* Max length of the platform information */
#define PTF_LEN        10    /* Max length of the ptf attached to a level */
#define QUI_LEN         3    /* Max length of the quiesce field */
#define QUI2_LEN        1    /* Max length of the quiesce field for SMIT */
#define TYPE_LEN        3    /* Max length of the I/U field */

/* added to determine who the caller is for the toc routines */

#define CALLER_OPEN_TOC   0    /* Called by the open_toc() routine */
#define CALLER_INUTOC     1    /* Called by the inutoc command */
#define CALLER_INSTALLP   2    /* Called by the installp command */
#define CALLER_INURECV    3    /* Called by the inurecv command */
#define CALLER_BFFCREATE  4    /* Called by the bffcreate command */
#define CALLER_INSTFIX    5    /* Called by the instfix command */


#define MAX_AFREC_SIZE  4096 /* size of attribute file records --   */
                             /* for update.list & for archive.list  */
#define MAX_ATTRS       30   /* max number of attributes per record */
#define DFLT_SAVEDIR    "/usr/lpp/"
#define MAX_STR_LEN     20   /* Max string length for string[] array elems */

typedef int boolean;

#define INUREST_ATTEMPTED  "inurest_attempted"

/*-------------------------------------------------------------------------
**  Define enumerated V_flag_type (integer values 0-4) for verbosity
**-------------------------------------------------------------------------*/
typedef enum {V0, V1, V2, V3, V4} V_flag_type;

/*-------------------------------------------------------------------------
**  Message types (values should not conflict with verbosity values defined
**  in the enumerated type V_flag_type in instl_options.h)
**-------------------------------------------------------------------------*/
#define SUCC_MSG     10    /* Success message type */
#define PROG_MSG     11    /* Progress message type */
#define INFO_MSG     12    /* Informational message type */
#define WARN_MSG     13    /* Warning message type */
#define FAIL_MSG     14    /* Failure message type */

/*-------------------------------------------------------------------------
**  Output method (values should not conflict with verbosity values defined
**  in the enumerated type V_flag_type in instl_options.h OR with the
**  message type values above).
**------------------------------------------------------------------------*/
#define LOG_MSG      20    /* Send output to the install log only */
#define SCREEN_MSG   21    /* Send output to the screen only */
#define SCREEN_LOG   22    /* Send output to both the screen and the log */

#define DEV_NULL    999    /* Don't print the message */

#define LESSER_LEVEL -1    /* 1st level is LESSER than the second level   */
#define LEVEL_MATCH   0    /* 1st level is equivalent to the second level */
#define GREATER_LEVEL 1    /* 1st level is GREATER than the second level  */

#endif  /* _H_INUDEF */
