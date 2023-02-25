#!/usr/bin/sh -
# @(#)24	1.3  src/bos/usr/sbin/sliplogin/slip.login, cmdnet, bos411, 9428A410j 4/26/94 08:35:44
# 
# COMPONENT_NAME: CMDNET
# 
# FUNCTIONS: 
#
# ORIGINS: 26  27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	@(#)slip.login	5.1 (Berkeley) 7/1/90

#
# generic login file for a slip line.  sliplogin invokes this with
# the parameters:
#      1        2         3        4          5         6     7-n
#   slipunit ttyspeed loginname local-addr remote-addr mask opt-args
#
/usr/sbin/ifconfig sl$1 inet $4 $5 netmask $6 up > /dev/null 2>&1
strinfo -m | grep "\'slip\'" > /dev/null 2>&1 || \
			strload -m /usr/lib/drivers/slip > /dev/null 2>&1
exit
