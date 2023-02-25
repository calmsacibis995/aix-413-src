#! /usr/bin/bsh
# @(#)27 1.5  src/bos/usr/sbin/acct/nulladm.sh, cmdacct, bos411, 9428A410j 5/12/93 14:24:39
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

#	"nulladm name..."
#	"creates each named file mode 664"
#	"make sure owned by adm (in case created by root)"
for _file
do
	cp /dev/null $_file
	chmod -f 664 $_file
	chown -f adm.adm $_file
done
