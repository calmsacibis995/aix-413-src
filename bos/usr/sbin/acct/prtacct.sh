#! /usr/bin/bsh
# @(#)30 1.6  src/bos/usr/sbin/acct/prtacct.sh, cmdacct, bos411, 9428A410j 9/17/93 09:02:39
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
#	"print daily/summary total accounting (any file in tacct.h format)"
#       "prtacct [-f fields ] [-v] file [heading]"
#
#       "fields is in the form of a field specification for acctmerg (1)"
# D43200, ATT compatible
FIELDS="1-6,11-13,15-18"
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc
VFLAG=

usage ()
{
	if dspmsg acct.cat 22 "Usage: prtacct [-f fields ] [-v] file [heading]\n
"
	then :
	else echo "Usage: prtacct [-f fields ] [-v] file [heading]\n"
	fi
	exit 1
}

set -- `getopt f:v $*`
if [ $? != 0 ]
then
	usage
fi

for i in $*
do
	case $i in
	-f)	FIELDS="$2"; shift; shift;;
	-v)	VFLAG="$1"; shift;;
	--)	shift; break;;
	-*)	usage;;
	esac
done

if [ $# -eq 0 ]
then
	usage
elif [ ! -r "$1" ]
then
	if dspmsg acct.cat 70 "%s: %s not found\n" $0 $1
	then :
	else echo "prtacct: $1 not found\n"
	fi
	usage
fi
FILE="$1"
# Take all arguments after the file as part of the heading.
HEADING="$2"
while [ -n "$3" ]
do
	HEADING=$HEADING" $3"
	shift
done
(acctmerg -t $VFLAG -h"$FIELDS" <"$FILE"; acctmerg -p"$FIELDS" $VFLAG \
 <"$FILE") | pr -h "$HEADING"
