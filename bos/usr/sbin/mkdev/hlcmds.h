/* @(#)34	1.6.1.3  src/bos/usr/sbin/mkdev/hlcmds.h, cmdcfg, bos411, 9428A410j 11/12/93 16:04:04 */
/*
 *   COMPONENT_NAME: CMDDEV
 *
 *   FUNCTIONS: ERRSTR
 *		METHERR
 *		MSGSTR
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_HLCMDS
#define _H_HLCMDS

#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#define FALSE			0
#define TRUE			1

/*----------------------------- locking -------------------------------------*/
#define CONFIG_LOCK		"/etc/objrepos/config_lock"
#define LOCK_TIMEOUT		5

/*------------------------------ exit codes --------------------------------*/
#define EXIT_NOT		-1
#define EXIT_OK			0
#define EXIT_FATAL		1

/*------------------------------- default messages -------------------------*/
/* NOTE - the default messages for cmdcfg reside at the end of cfglib.c     */
extern char *cmdcfg_msg[];
extern char *cmdcfg_err[];

/*------------------------------- NLS macros ------------------------------*/
#include <nl_types.h>
#include <locale.h>
#define MSGSTR(num) \
        catgets( catopen(CFG_MSG_CATALOG,NL_CAT_LOCALE), CFG_MSG_SET, num, cmdcfg_msg[num])
#define ERRSTR(num) \
        catgets( catopen(CFG_MSG_CATALOG,NL_CAT_LOCALE), CFG_ERR_SET, num, cmdcfg_err[num])
#define METHERR(num) \
        catgets( catopen(CFG_MSG_CATALOG,NL_CAT_LOCALE), CFG_METH_SET, num, meth_err_msg[num])

/*------------------------------ messages ----------------------------------*/
 /* $  Do not translate any %s or (%s) found in this message catalog. $ */
/* $  These messages are to be used by the system manager for maintaining \n\
 the hardware devices and pseudodevices attached to his computer.  He must \n\
 have a certain basis of knowledge about the how this is accomplished and \n\
 with the vocabulary, especially the concepts of the ODM, class, and \n\
 and object. $ */
/* $ If some of the error messages seem generic, it is because in our \n\
 system management architecture the routine which reports the error is a \n\
 different process from the command which detected it, and it has no way \n\
 to know the details of the error. */

	/* $  This set contains only information messages. $ */

#define CFG_MSG_FALSE		1  /* "False" */
#define CFG_MSG_DEFINED		2  /* "Defined" */
#define CFG_MSG_STOPPED		3  /* "Stopped" */
#define CFG_MSG_AVAILABLE	4  /* "Available" */
#define CFG_MSG_UNKNOWN		5  /* "Unknown" */
#define CFG_DDEFINED		6 /* "%s Defined\n" */
#define CFG_DSTOPPED		7 /* "%s Stopped\n" */
#define CFG_DAVAILABLE		8 /* "%s Available\n" */
#define CFG_D_CHANGED		9 /* "%s changed\n" */
#define CFG_MSG_DELETED		10 /* "%s deleted\n" */
#define IP_SUCCESS		11  /* "%d bytes were written to address 0x%x in %s\n" */

#define NVL_SUCCESS		12  /* "%s loaded into %s\n" */
#define TOO_MANY_ATTRS		13  /* "too may attributes" */



#define CFG_MKDEV_USAGE		14 /* \
	"Usage:\n\
mkdev {[-c Class][-s Subclass][-t Type]}[-a Attribute=Value]...\n\
\t[-d|-S][-p ParentName][-q][-w ConnectionLocation][-f File][-]\n\
mkdev -l Name [-h][-q][-S][-f File][-]\n\
mkdev -h\n" */

#define CFG_CHDEV_USAGE		15 /* \
	"Usage:\n\
chdev -l Name [-a Attribute=Value]...[-p ParentName][-P|-T]\n\
\t[-q][-w ConnectionLocation][-f File][-]\n\
chdev -h\n" */

#define CFG_RMDEV_USAGE		16 /* \
	"Usage:\n\
\trmdev -l name[-d|-S][-q][-f File][-]\n\
\trmdev -h\n" */

#define CFG_LSATTR_USAGE	17 /* \
"Usage:\n\
lsattr {-D[-O]| -E[-O] | -F Format} -l Name [-a Attribute]...[-H]\n\
\t[-f File][-]\n\
lsattr {-D[-O]| -F Format}{[-c Class][-s Subclass][-t Type]}[-a Attribute]...\n\
\t[-H][-f File][-]\n\
lsattr -R {-l Name | [-c Class][-s Subclass][-t Type]} -a Attribute [-H]\n\
\t[-f File][-]\n\
lsattr -h\n" */

#define CFG_LSCONN_USAGE	18 /* \
"Usage:\n\
lsconn -p ParentName [-l ChildName | -k ChildConnectionKey] [-F format][-H]\n\
\t[-f file][-]\n\
lsconn {[-c ParentClass] [-s ParentSubclass] [-t ParentType]}\n\
\t[-l ChildName | -k ChildConnectionKey] [-F format][-H][-f file][-]\n\
lsconn -h\n" */

#define CFG_LSDEV_USAGE		19 /* \
"Usage:\n\
lsdev -C [-c Class][-s Subclass][-t Type][-S State][-l Name]\n\
\t[-r ColumnName| -F Format][-H][-f File ][-]\n\
lsdev -P [-c Class][-s Subclass][-t Type][-r ColumnName| -F Format]\n\
\t[-H][-f File ][-]\n\
lsdev  -h\n" */

#define CFG_LSPARENT_USAGE	20 /* \
"Usage:\n\
lsparent -P {-k ChildConnectionKey | -l ChildName}[-F format][-H][-f file][-]\n\
lsparent -C {-k ChildConnectionKey | -l ChildName}[-F format][-H][-f file][-]\n\
lsparent -h\n" */

#define CFG_NVL_USAGE		21 /* \
"Usage:\n\
\tnvload [-f filename] [-n section] [-t string]\n" */

#define CFG_MSG_TRUE		22  /* "True" */
#define CFG_MSG_DIAGNOSE        23  /* "Diagnose" state */

/*--------------------- High Level Command Specific Errors -----------------*/

#define CFG_ARGOROPT_CONFLICT	1  
#define CFG_ARGOROPT_MISSING	2  
#define CFG_OPT_ARG_MISSING	3  
#define CFG_OPEN_ERR				4  
#define CFG_STATE_CONFLICT		5  
#define CFG_MALLOC_FAILED		6   
#define CFG_DISPLAYABLE_ERR   7   
#define CFG_SAVEBASE_FAILED   8 
#define NVL_SYSTEM_ERR			9  
#define NVL_WRITE_ERR			10  
#define NVL_READ_ERR				11  
#define NVL_MAX_DD				12  
#define NVL_SECTION				13 
#define NVL_SECTION_CONFLICT 	14 
#define NVL_NO_DD					15 
#define CFG_ODM_LOCK				16  
#define CFG_ODM_NONUNIQUE		17 
#define CFG_ODM_ACCESS			18 
#define CFG_ODM_NOCUDV			19 
#define CFG_ODM_NOPDDV			20 
#define CFG_ODM_NOCUPD			21 
#define CFG_ODM_RUN_METHOD		22 
#define CFG_ODM_NO_DEFINE		23 
#define CFG_ODM_NO_CONFIGURE	24 
#define CFG_ODM_NO_CHANGE		25 
#define CFG_ODM_NO_UNDEFINE	26 
#define CFG_ODM_NO_START		27 
#define CFG_ODM_NOPDAT			28
#define CFG_ODM_COL_ERR			29
#define CFG_BAD_ATTR				30
#define CFG_INTERNAL_ERR		31
#define CFG_METHERR				32
#define CFG_MISC_SYNTAX_ERR	33  
#define CFG_DEV_DIAGNOSE        34
#define CFG_PAR_DIAGNOSE        35

#endif   /* endif _H_HLCMDS */
