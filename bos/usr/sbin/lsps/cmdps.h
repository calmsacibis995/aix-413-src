/* @(#)69	1.5.1.1  src/bos/usr/sbin/lsps/cmdps.h, cmdps, bos411, 9428A410j 6/2/93 09:57:08 */
#ifndef _h_CMDPS
#define _h_CMDPS

/*
 * COMPONENT_NAME: (CMDPS) paging space commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <ctype.h>
#include <sys/types.h>
#include <locale.h>
    
#define	getenv	NLgetenv

/* Default messages (from cmdps.msg).
 */
static char *msgstr[] =
{
"",
"%1$s: Specify volume group name.\n",
"%1$s: Specify paging space logical volume name.\n",
"%1$s: Specify physical volume name.\n",
"%1$s: Specify size.\n",
"%1$s: The parameter for the -a flag\n\
\tmust be character y or n.\n",
"%1$s: The parameter for NewLPs\n\
\tmust be greater than zero.\n",
"%1$s: %2$s is not a paging space logical volume.\n",
"%1$s: The parameter -s can not be \n\
\tspecified for an NFS paging space.\n",
"%1$s: Command name should be either\n\
\tchps, lsps, mkps, or rmps -- lsps is the default.\n",
"%1$s: Cannot access or update file %2$s.\n",
"%1$s: Cannot activate paging space %2$s.\n",
"Usage: %1$s [-s NewLPs] [-a {y|n}] Psname\n\
\tChanges attributes of a paging space.\n",
"%1$s: Cannot change paging space %2$s.\n",
"Usage: %1$s [-c | -l] {-a | Psname | -t {lv|nfs} }\n\
\tLists the characteristics of a paging space.\n",
"%1$s: Cannot list paging space %2$s.\n",
"%1$s: Specify either the -a flag or Psname.\n",
"%1$s: Do not specify the -c flag with the -l flag.\n",
"%1$s: Do not specify the -a flag with Psname.\n",
"#Psname:Pvname:Vgname:Size:Used:Active:Auto:Type\n",
"%1$s:%2$s:%3$s:%4$d:%5$d:%6$s:%7$s:%8$s\n",
"Page Space  Physical Volume   Volume Group    Size   %%Used  Active  Auto  Type\n",
"%-12s%-18s%-12s%6dMB%8d%8s%6s%6s\n",
"#Psname:Hostname:Filename:Size:Used:Active:Auto:Type\n",
"%1$s:%2$s:%3$s:%4$d:%5$d:%6$s:%7$s:%8$s\n",
"Page Space  Server Hostname   File Name       Size   %%Used  Active  Auto  Type\n",
"%-12s%-18s%-12s%6dMB%8d%8s%6s%6s\n",
"yes",
"no",
"Usage: %1$s [-a] [-n] [-t lv] -s NumLPs Vgname Pvname\n\
\tMakes a paging space using logical volume.\n\
%1$s [-a] [-n] -t nfs hostname pathname\n\
\tMakes a paging space using an NFS server\n",
"%1$s: Cannot make paging space.\n",
"Usage: %1$s Psname\n\
\tRemoves a paging space.\n",
"%1$s: Cannot remove paging space %2$s.\n",
"%1$s: Paging space %2$s is active.\n",
"Usage: %1$s {-a | DeviceName...}\n\
\tActivates a paging device.\n",
"%1$s: Specify either the -a flag or DeviceName.\n",
"%1$s: Cannot activate paging device %2$s.\n",
"%1$s: Paging device %2$s activated.\n",
"Usage: %1$s {-s | [-c | -l] {-a | Psname | -t {lv|nfs} } }\n\
\tLists the characteristics of a paging space.\n",
"Total Paging Space   Percent Used\n",
};

#ifdef MSG

#include <nl_types.h>
#include "cmdps_msg.h"
    
static nl_catd	nls_catd;

#define MSGSTR(num)	NLcatgets (nls_catd, MS_CMDPS, num, msgstr[num])
    
#else

/* defines from cmdps_msg.h
 */
#define NO_VG 1
#define NO_PS 2
#define NO_PV 3
#define NO_SIZE 4
#define BAD_APARM 5
#define BAD_SPARM 6
#define BAD_PSNAME 7
#define BAD_SPARM_NFS 8
#define WARN_NAME 9
#define WARN_AUTO 10
#define WARN_SWAPON 11
#define CHPS_USAGE 12
#define CHPS_FAIL 13
#define NOT_USED 14
#define LSPS_FAIL 15
#define LSPS_AORPS 16
#define LSPS_CANDL 17
#define LSPS_AANDPS 18
#define LSPS_CHEADER 19
#define LSPS_CFORMAT 20
#define LSPS_HEADER 21
#define LSPS_FORMAT 22
#define LSPS_CHEADER_NFS 23
#define LSPS_CFORMAT_NFS 24
#define LSPS_HEADER_NFS 25
#define LSPS_FORMAT_NFS 26
#define LSPS_YES 27
#define LSPS_NO 28
#define MKPS_USAGE 29
#define MKPS_FAIL 30
#define RMPS_USAGE 31
#define RMPS_FAIL 32
#define RMPS_ACTIVE 33
#define SWAPON_USAGE 34
#define SWAPON_AORDN 35
#define SWAPON_FAIL 36
#define SWAPON_ADD 37
#define LSPS_USAGE 38
#define LSPS_SUMHEADER 39

#define	MSGSTR(num)	msgstr[num]

#endif /* MSG */

#define NILSTR		""
#define NIL(type)	((type) 0)
#define NILPTR(type)	((type *) 0)

/* Default paging space stanza file and environment variable.
 */
#define ETCSWAP		"/etc/swapspaces"
#define ENVSWAP		"SWAPSPACES"

/* Variables used by getopt()
 */
extern int optind;
extern char *optarg;
extern int optopt;
extern int opterr;

#endif /* _h_CMDPS */
