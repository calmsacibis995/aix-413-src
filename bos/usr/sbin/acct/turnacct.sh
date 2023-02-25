#! /usr/bin/bsh
# @(#)41 1.3.1.2  src/bos/usr/sbin/acct/turnacct.sh, cmdacct, bos411, 9428A410j 5/12/93 13:50:49
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

#	"control process accounting (must be root)"
#	"turnacct on	makes sure it's on"
#	"turnacct off	turns it off"
#	"turnacct switch	switches pacct to pacct?, starts fresh one"
#	"/var/adm/pacct is always the current pacct file"
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc
cd /var/adm
case "$1"  in
on)
	if test ! -r pacct
	then
		nulladm pacct
	fi
	accton pacct
	_rc=$?
	;;
off)
	accton
	_rc=$?
	;;
switch)
	if test -r pacct
	then
		_i=1
		while test -r pacct${_i}
		do
			_i="`expr ${_i} + 1`"
		done
		mv pacct pacct${_i}
	fi
	nulladm pacct
	accton
	accton pacct
	_rc=$?
	if test ${_rc} -ne 0; then
		echo "accton failed"
		rm pacct
		mv pacct${_i} pacct
		exit ${_rc}
	fi
	;;
*)
	if dspmsg acct.cat 64 "Usage: turnacct on|off|switch\n"; then :
	else echo "Usage: turnacct on|off|switch\n"; fi
	_rc=1
	;;
esac
exit ${_rc}
