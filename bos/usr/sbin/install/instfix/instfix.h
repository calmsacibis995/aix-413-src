/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: instfix.h
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_INSTFIX
#define _H_INSTFIX

/* NAME: instfix.h
 *
 *
 * FUNCTION: This header contains the structure used throughout the installp
 *           command and its initialization.
 *
 * FLAGS :
 *
 *   d <device>    - device to be used during install.
 *   k             - keyword to install
 *   f             - input file
 *   i             - information only
 *   T             - List keywords in the TOC
 *   s             - limit list output to toc entries containing string
 *   p             - output filesets to stdout without invoking installp
 *   a             - show abstract and symptom information when using -i
*/

#define KEYWORDSIZE  16
#define d_FLAG	     instfix.d_flag
#define f_FLAG	     instfix.f_flag
#define i_FLAG	     instfix.i_flag
#define k_FLAG	     instfix.k_flag
#define s_FLAG	     instfix.s_flag
#define T_FLAG	     instfix.T_flag
#define p_FLAG	     instfix.p_flag
#define a_FLAG	     instfix.a_flag
#define c_FLAG	     instfix.c_flag
#define F_FLAG	     instfix.F_flag
#define q_FLAG	     instfix.q_flag
#define v_FLAG	     instfix.v_flag
#define t_FLAG	     instfix.t_flag


int inu_putenv (char *);

int inu_rm (char *);
int vpdlocalpath (char *);
int vpdlocal();

typedef enum compare {NOT_INSTALLED, DOWN_LEVEL, EXACT_MATCH, SUPERSEDED} cmp;

/****************************
 * Define instfix structure
 ****************************/

struct FLAGS
{
    char            d_flag;      /* device to use */
    char            f_flag;      /* input file to use */
    char            i_flag;      /* information query only */
    char            k_flag;      /* keyword to use */
    char            s_flag;      /* use string for TOC search */
    char            T_flag;      /* list TOC option */
    char            p_flag;      /* do not invoke installp */
    char            a_flag;      /* include symptom string in output */
    char            c_flag;      /* colon-separated output */
    char            q_flag;      /* quiet flag */
    char            v_flag;      /* verbose output */
    char            F_flag;      /* Full set of filesets required */
    char            t_flag;      /* keyword type to use */
    char DEVICE[PATH_MAX+1];     /* device name */
    char KEYWORDS[MAX_INPUT+1];   /* keywords */
    char STRING[81];		 /* string for TOC search */
    char KEYTYPE[2];		 /* keytype for VPD search */
    char TMPDIR[PATH_MAX+1];     /* temporary directory */
    char INFILE[PATH_MAX+1];     /* input file */
};


#ifdef MAIN_PROGRAM

struct FLAGS instfix = { /* intialize the structure */

/* instfix.d_flag = */	FALSE_C,
/* instfix.f_flag = */	FALSE_C,
/* instfix.i_flag = */	FALSE_C,
/* instfix.k_flag = */	FALSE_C,
/* instfix.s_flag = */	FALSE_C,
/* instfix.T_flag = */	FALSE_C,
/* instfix.p_flag = */	FALSE_C,
/* instfix.a_flag = */	FALSE_C,
/* instfix.c_flag = */	FALSE_C,
/* instfix.v_flag = */	FALSE_C,
/* instfix.q_flag = */	FALSE_C,
/* instfix.F_flag = */	FALSE_C,
/* instfix.t_flag = */	FALSE_C,
/* instfix.DEVICE = */	"",
/* instfix.KEYWORDS= */ "",
/* instfix.STRING = */	"",
/* instfix.KEYTYPE = */ "",
/* instfix.TMPDIR = */	"",
/* instfix.INFILE= */	""

};/* end initialization */

#else
extern struct FLAGS instfix;

#endif

#endif  /* _H_INSTFIX */
