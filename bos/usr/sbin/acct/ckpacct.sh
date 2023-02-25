#! /usr/bin/bsh
# @(#)19 1.6  src/bos/usr/sbin/acct/ckpacct.sh, cmdacct, bos411, 9428A410j 5/12/93 13:45:06
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
#       "periodically check the size of /var/adm/pacct"
#       "if over $1 blocks (500) default, execute turnacct switch"
#       "should be run as adm"

# MAILCOM = "mail root adm" to send mail to root adm
MAILCOM=${MAILCOM:-':'}
ACCTDIR=${ACCTDIR:-/var/adm}
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc
trap "rm -f /var/adm/cklock*; exit 0" 0 1 2 3 9 15
_max=${1-500}
_MIN_BLKS=1000
cd $ACCTDIR

#	set up lock files to prevent simultaneous checking

cp /dev/null cklock
chmod 400 cklock
ln cklock cklock1
if test $? -ne 0 ; then exit 1; fi

#       If there are less than $_MIN_BLKS free blocks left on the /var/adm
#	file system, turn off the accounting (unless things improve
#	the accounting wouldn't run anyway).  If something has
#	returned the file system space, restart accounting.  This
#	feature relies on the fact that ckpacct is kicked off by the
#	cron at least once per hour.

_blocks=`df /var/adm | awk 'NR>1{ print $3 }'`
if [ "$_blocks" -lt $_MIN_BLKS   -a  -f /tmp/acctoff ];then
	if dspmsg acct.cat 3 "ckpacct: /var/adm still low on space (%s blks); acctg still off\n" $_blocks
	then
	(dspmsg acct.cat 3 "ckpacct: /var/adm still low on space (%s blks); acctg still off\n" $_blocks) | $MAILCOM
	else
	echo "ckpacct: /var/adm still low on space ($_blocks blks); \c"
	echo "acctg still off"
	( echo "ckpacct: /var/adm still low on space ($_blocks blks); \c"
	echo "acctg still off" ) | $MAILCOM
	fi
	exit 1
elif [ "$_blocks" -lt $_MIN_BLKS ];then
	if dspmsg acct.cat 4 "ckpacct: /var/adm too low on space (%s blks);  turning acctg off\n" $_blocks
	then
	(dspmsg acct.cat 4 "ckpacct: /var/adm too low on space (%s blks);  turning acctg off\n" $_blocks) | $MAILCOM
	else
	echo "ckpacct: /var/adm too low on space ($_blocks blks); \c"
	echo "turning acctg off"
	( echo "ckpacct: /var/adm too low on space ($_blocks blks); \c"
	echo "turning acctg off" ) | $MAILCOM
	fi
	nulladm /tmp/acctoff
 	turnacct off
	exit 1
elif [ -f /tmp/acctoff ];then
	if dspmsg acct.cat 8 "ckpacct: /var/adm free space restored; turning acctg on\n"
	then
	(dspmsg acct.cat 8 "ckpacct: /var/adm free space restored; turning acctg on\n") | $MAILCOM
	else
	echo "ckpacct: /var/adm free space restored; turning acctg on"
	echo "ckpacct: /var/adm free space restored; turning acctg on" | $MAILCOM
	fi
	rm /tmp/acctoff
	turnacct on
fi

_cursize="`du -s pacct | sed 's/	.*//'`"
if [ "${_max}" -lt "${_cursize}" ]; then
	turnacct switch
fi
