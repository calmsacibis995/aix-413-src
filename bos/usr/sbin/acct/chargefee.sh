#! /usr/bin/bsh
# @(#)18 1.5  src/bos/usr/sbin/acct/chargefee.sh, cmdacct, bos411, 9428A410j 5/12/93 13:41:52
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
#       "chargefee login-name number"
#	"emits tacct.h/ascii record to charge name $number"

ACCTDIR=${ACCTDIR:-/var/adm}
cd $ACCTDIR
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc

if test $# -lt 2; then
	if dspmsg acct.cat 65 "usage: chargefee name number\n"
	then :
	else echo "usage: chargefee name number\n"
	fi
	exit
fi

_entry="`grep \^$1: /etc/passwd`"         # get login name info from /etc/passwd
if test -z "${_entry}"; then   		  # if no info from /etc/passwd check yp
	_domain="`domainname`"		  # check to see if yp is running
	if test -z "${_domain}"; then	  # if yp is not running 
					  #     echo " can't find login name"
		if dspmsg acct.cat 1 "can't find login name ";
		then :	
		else echo "can't find login name "; fi
		echo "$1"
		exit			     
	fi
	_entry="`ypmatch $1 passwd.byname`"  # get login name info from ypmatch
	if test $? -eq 1; then		     # test for a failed match 
					     #     echo " can't find login name"
		if dspmsg acct.cat 1 "can't find login name ";
		then :	
		else echo "can't find login name "; fi
		echo "$1"
		exit
	fi
	if test -z "${_entry}"; then	    # if no info from ypmatch
		if dspmsg acct.cat 1 "can't find login name ";
		then :	
		else echo "can't find login name "; fi
		echo "$1"
		exit
	fi
fi

amt="`expr "$2".00 : '\(-\{0,1\}[0-9]*\.[0-9][0-9]*\)\.\{0,1\}[0-9]*$'`"
if test -z "$amt" ; then
	if dspmsg acct.cat 2 "charge invalid: "; 
	then :
	else echo "charge invalid: "; fi
	echo "$2"
	exit
fi

if test ! -r fee; then
	nulladm fee
fi

_userid=`echo "${_entry}" | cut -d: -f3`  # get the UID
echo  "${_userid} $1 0 0 0 0 0 0 0 0 0 0 0 0 $2 0 0 0" >>fee
