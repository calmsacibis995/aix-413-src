#!/usr/bin/bsh
# @(#)29 1.4  src/bos/usr/sbin/acct/prdaily.sh, cmdacct, bos411, 9428A410j 5/12/93 13:36:00
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

#	"prdaily	prints daily report"
#	"last command executed in runacct"
#	"if given a date mmdd, will print that report"
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc

set -- `getopt cl $*`
if [ $? != 0 ]
then
	if dspmsg acct.cat 15  "Usage: prdaily [-c] [-l] [mmdd]\n"
	then :
	else echo  "Usage: prdaily [-c] [-l] [mmdd]\n"
	fi
	exit 2
fi
for i in $*
do
	case $i in
	-c)	CMDEXCPT=1; shift;;
	-l)	LINEEXCPT=1; shift;;
	--)	shift; break;;
	esac
done
date=`date +%m%d`
_sysname="`uname`"
_lib=/usr/sbin/acct
_adm=/var/adm
_nite=/acct/nite
_sum=${_adm}/acct/sum

cd ${ACCTDIR:-${_adm}}${_nite}
if [ `expr "$1" : [01][0-9][0-3][0-9]` -eq 4 -a "$1" != "$date" ]; then
	if [ "$CMDEXCPT" = "1" ]
	then
		date1=`date '+%h %d'`
		if dspmsg acct.cat 16  "Cannot print command exception reports except for %s\n" "$date1"
		then :
		else echo "Cannot print command exception reports except for $date1\n" 
		fi
		exit 5
	fi
	if [ "$LINEEXCPT" = "1" ]
	then
		acctmerg -a < ${_sum}/tacct$1 | awk -f ${_lib}/ptelus.awk
		exit $?
	fi
	cat ${_sum}/rprt$1
	exit 0
fi

if [ "$CMDEXCPT" = 1 ]
then
	acctcms -a -s ${_sum}/daycms | awk -f ${_lib}/ptecms.awk
fi
if [ "$LINEEXCPT" = 1 ]
then
	acctmerg -a < ${_sum}/tacct${date} | awk -f ${_lib}/ptelus.awk
fi
if [ "$CMDEXCPT" = 1 -o "$LINEEXCPT" = 1 ]
then
	exit 0
fi
if HDR=`dspmsg acct.cat 17 'DAILY REPORT FOR %s\n' ${_sysname}`; then :
else HDR="DAILY REPORT FOR ${_sysname}"; fi
if HDR1=`dspmsg acct.cat 18 'DAILY USAGE REPORT FOR %s\n' ${_sysname}`; then :
else HDR1="DAILY USAGE REPORT FOR ${_sysname}"; fi
if HDR2=`dspmsg acct.cat 19 'DAILY COMMAND SUMMARY'`; then :
else HDR2="DAILY COMMAND SUMMARY"; fi
if HDR3=`dspmsg acct.cat 20 'MONTHLY TOTAL COMMAND SUMMARY'`; then :
else HDR3="MONTHLY TOTAL COMMAND SUMMARY"; fi
if HDR4=`dspmsg acct.cat 21 'LAST LOGIN'`; then :
else HDR4="LAST LOGIN"; fi

(cat reboots; echo ""; cat lineuse) | pr -h "$HDR"

prtacct daytacct "$HDR1"
pr -h "$HDR2" daycms
pr -h "$HDR3" cms
pr -h "$HDR4" -3 ../sum/loginlog  
