#! /usr/bin/bsh
# @(#)26 1.4  src/bos/usr/sbin/acct/monacct.sh, cmdacct, bos411, 9428A410j 5/12/93 14:24:36
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
#	"this procedure should be run periodically ( by month or fiscal )"
_adm=/var/adm
_sum=${_adm}/acct/sum
_fiscal=${_adm}/acct/fiscal
PATH=:/usr/sbin/acct:/bin:/usr/bin:/etc

_period=${1-`date +%m`}

cd ${_adm}

#	"move summary tacct file to fiscal directory"
mv ${_sum}/tacct ${_fiscal}/tacct${_period}

#	"delete the daily tacct files"
rm -f ${_sum}/tacct????

#	"restart summary tacct file"
nulladm ${_sum}/tacct

#	"move summary cms file to fiscal directory
mv ${_sum}/cms ${_fiscal}/cms${_period}

#	"restart summary cms file"
nulladm ${_sum}/cms

#	"remove old prdaily reports"
rm -f ${_sum}/rprt*

#	"produce monthly reports"
prtacct ${_fiscal}/tacct${_period} > ${_fiscal}/fiscrpt${_period}

if _hperiod="`dspmsg acct.cat 66 'TOTAL COMMAND SUMMARY FOR FISCAL'`"
then
	:
else
	_hperiod="TOTAL COMMAND SUMMARY FOR FISCAL"
fi

if _hlogin="`dspmsg acct.cat 67 'LAST LOGIN'`"
then
	:
else
	_hlogin="LAST LOGIN"
fi

acctcms -a -s ${_fiscal}/cms${_period} |  \
pr -h "${_hperiod} ${_period}" >> ${_fiscal}/fiscrpt${_period}
pr -h "${_hlogin}" -3 ${_sum}/loginlog >> ${_fiscal}/fiscrpt${_period}

#	"add commands here to do any charging of fees, etc"
