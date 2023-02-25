#! /usr/bin/bsh
# @(#)28 1.4  src/bos/usr/sbin/acct/prctmp.sh, cmdacct, bos411, 9428A410j 5/12/93 14:24:42
#
#  COMPONENT_NAME: (CMDACCT) Command Accounting
# 
#  FUNCTIONS: none
# 
#  ORIGINS: 3, 9, 27
# 
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
#  All Rights Reserved
#  Licensed Materials - Property of IBM
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
#	"print session record file (ctmp.h/ascii) with headings"
#	"prctmp file [heading]"
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc
if HDR=`dspmsg acct.cat 14 "SESSIONS, SORTED BY ENDING TIME"`
then :
else HDR="SESSIONS, SORTED BY ENDING TIME"
fi
(if dspmsg   acct.cat 12 "MAJ/MIN			CONNECT SECONDS	START TIME	SESSION START\n"
then dspmsg acct.cat 13 "DEVICE	UID	LOGIN	PRIME	NPRIME	(NUMERIC)	DATE	TIME\n"
else echo "MAJ/MIN			CONNECT SECONDS	START TIME	SESSION START\n"
echo "DEVICE	UID	LOGIN	PRIME	NPRIME	(NUMERIC)	DATE	TIME\n"
fi
cat $*) | pr -h "$HDR"
