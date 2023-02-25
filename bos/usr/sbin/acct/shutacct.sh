#! /usr/bin/bsh
# @(#)37 1.4  src/bos/usr/sbin/acct/shutacct.sh, cmdacct, bos411, 9428A410j 5/12/93 14:24:59
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

#	"shutacct [arg] - shuts down acct, called from /etc/shutdown"
#	"whenever system taken down"
#       "arg    added to /var/adm/wtmp to record reason, defaults to shutdown"
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc
if _def_reason="`dspmsg acct.cat 68 'shutdown,acctg off'`"
then
	:
else
	_def_reason="shutdown,acctg off"
fi
_reason=${1-${_def_reason}}
acctwtmp  "${_reason}"  >> /var/adm/wtmp
turnacct off
