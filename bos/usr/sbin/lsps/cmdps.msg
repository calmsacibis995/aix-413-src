$ @(#)66	1.5.1.1  src/bos/usr/sbin/lsps/cmdps.msg, cmdps, bos411, 9428A410j 6/2/93 10:01:52
$
$ COMPONENT_NAME: (CMDPS) paging space commands
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$set MS_CMDPS

$ $ Message component 0517 for the commands chps, lsps, mkps, rmps, swapon
$ Variable 1 in most messages is the command name.

$ $ Parameter required messages -- range 001-009

NO_VG		"0517-001 %1$s: Specify volume group name.\n"
NO_PS		"0517-002 %1$s: Specify paging space logical volume name.\n"
NO_PV		"0517-003 %1$s: Specify physical volume name.\n"
NO_SIZE		"0517-004 %1$s: Specify size.\n"

$ $ Parameter invalid messages -- range 010-019
$ Message 012: variable 2 is a paging space name.

BAD_APARM	"0517-010 %1$s: The parameter for the -a flag\n\
\tmust be character y or n.\n"
BAD_SPARM	"0517-011 %1$s: The parameter for NewLPs\n\
\tmust be greater than zero.\n"
BAD_PSNAME	"0517-012 %1$s: %2$s is not a paging space logical volume.\n"
BAD_SPARM_NFS	"0517-013 %1$s: The parameter -s can not be \
\tspecified for an NFS paging space.\n"

$ $ Warning messages -- range 020-029
$ Message 021: variable 2 is a file name.
$ Message 022: variable 2 is a paging space name.

WARN_NAME	"0517-020 %1$s: Command name should be either\n\
\tchps, lsps, mkps, or rmps -- lsps is the default.\n"
WARN_AUTO	"0517-021 %1$s: Cannot access or update file %2$s.\n"
WARN_SWAPON 	"0517-022 %1$s: Cannot activate paging space %2$s.\n"

$ $ Messages specific to the individual commands

$ $ chps messages -- range 030-039
$ Message 031: variable 2 is a paging space name.

CHPS_USAGE 	"0517-030 Usage: %1$s [-s NewLPs] [-a {y|n}] Psname\n\
\tChanges attributes of a paging space.\n"
CHPS_FAIL 	"0517-031 %1$s: Cannot change paging space %2$s.\n"

$ $ lsps messages	-- range 040-049
$ Message 041: variable 2 is a paging space name.
$ Do NOT translate messages LSPS_CHEADER, LSPS_CFORMAT
$ The column headers in message LSPS_HEADER should fit on one line and
$ should line up with data formatted with LSPS_FORMAT

NOT_USED 	"0517-040 Usage: %1$s [-c | -l] {-a | Psname | -t {lv|nfs} }\n\
\tLists the characteristics of a paging space.\n"
LSPS_FAIL 	"0517-041 %1$s: Cannot list paging space %2$s.\n"
LSPS_AORPS 	"0517-042 %1$s: Specify either the -a flag or Psname.\n"
LSPS_CANDL 	"0517-043 %1$s: Do not specify the -c flag with the -l flag.\n"
LSPS_AANDPS 	"0517-044 %1$s: Do not specify the -a flag with Psname.\n"
LSPS_CHEADER	"#Psname:Pvname:Vgname:Size:Used:Active:Auto:Type\n"
LSPS_CFORMAT	"%1$s:%2$s:%3$s:%4$d:%5$d:%6$s:%7$s:%8$s\n"
LSPS_HEADER	"Page Space  Physical Volume   Volume Group    Size   %%Used  Active  Auto  Type\n"
LSPS_FORMAT	"%-12s%-18s%-12s%6dMB%8d%8s%6s%6s\n"
LSPS_CHEADER_NFS	"#Psname:Hostname:Filename:Size:Used:Active:Auto:Type\n"
LSPS_CFORMAT_NFS	"%1$s:%2$s:%3$s:%4$d:%5$d:%6$s:%7$s:%8$s\n"
LSPS_HEADER_NFS	"Page Space  Server Hostname   File Name       Size   %%Used  Active  Auto  Type\n"
LSPS_FORMAT_NFS	"%-12s%-18s%-12s%6dMB%8d%8s%6s%6s\n"
LSPS_YES	"yes"
LSPS_NO		"no"

$ $ mkps messages -- range 050-059

MKPS_USAGE 	"0517-050 Usage: %1$s [-a] [-n] [-t lv] -s NumLPs Vgname Pvname\n\
\tMakes a paging space using logical volume.\n\
%1$s [-a] [-n] -t nfs hostname pathname\n\
\tMakes a paging space using an NFS server\n"
MKPS_FAIL 	"0517-051 %1$s: Cannot make paging space.\n"
	
$ $ rmps messages -- range 060-069
$ Message 061: variable 2 is a paging space name.
$ Message 062: variable 2 is a paging space name.

RMPS_USAGE 	"0517-060 Usage: %1$s Psname\n\
\tRemoves a paging space.\n"
RMPS_FAIL 	"0517-061 %1$s: Cannot remove paging space %2$s.\n"
RMPS_ACTIVE 	"0517-062 %1$s: Paging space %2$s is active.\n"

$ $ swapon messages -- range 070-079
$ Message 072: variable 2 is a device name.
$ Message SWAPON_ADD: variable 2 is a device name.

SWAPON_USAGE 	"0517-070 Usage: %1$s {-a | DeviceName...}\n\
\tActivates a paging device.\n"
SWAPON_AORDN 	"0517-071 %1$s: Specify either the -a flag or DeviceName.\n"
SWAPON_FAIL 	"0517-072 %1$s: Cannot activate paging device %2$s.\n"
SWAPON_ADD	"%1$s: Paging device %2$s activated.\n"

$ $ New lsps messages
$ The column headers in message LSPS_SUMHEADER should either be the same
$ width as the existing headers or they should be centered in the
$ space used by the headers below.
LSPS_USAGE 	"0517-040 Usage: %1$s {-s | [-c | -l] {-a | Psname | -t {lv|nfs} } }\n\
\tLists the characteristics of a paging space.\n"
LSPS_SUMHEADER	"Total Paging Space   Percent Used\n"
