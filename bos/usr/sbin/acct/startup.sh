#! /usr/bin/bsh
# @(#)38 1.4  src/bos/usr/sbin/acct/startup.sh, cmdacct, bos411, 9428A410j 5/12/93 14:25:02
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

#	"startup (acct) - should be called from /etc/rc"
#	"whenever system is brought up"
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc
MSG="`uname`, `dspmsg -s 1 acct.cat 63 'acctg on'`"
acctwtmp "$MSG" >> /var/adm/wtmp
turnacct on
remove
