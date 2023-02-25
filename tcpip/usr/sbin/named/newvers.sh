#!/bin/sh -
# @(#)03	1.7  src/tcpip/usr/sbin/named/newvers.sh, tcpnaming, tcpip411, GOLD410 9/3/93 11:49:28
# 
# COMPONENT_NAME: TCPIP newvers.sh
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that the above copyright notice and this paragraph are
# duplicated in all such forms and that any documentation,
# advertising materials, and other materials related to such
# distribution and use acknowledge that the software was developed
# by the University of California, Berkeley.  The name of the
# University may not be used to endorse or promote products derived
# from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#	newvers.sh	4.5 (Berkeley) 7/9/88
#
ECHO=$ODE_TOOLS/usr/bin/echo
TOUCH=$ODE_TOOLS/usr/bin/touch
RM=$ODE_TOOLS/usr/bin/rm
CAT=$ODE_TOOLS/usr/bin/cat
DATE=$ODE_TOOLS/usr/bin/date
SED=$ODE_TOOLS/usr/bin/sed
EXPR=$ODE_TOOLS/usr/bin/expr
PWD=$ODE_TOOLS/usr/bin/pwd
_HOSTNAME=$ODE_TOOLS/usr/bin/hostname

if [ ! -r version ]
then
	$ECHO 0 > version
fi
$TOUCH version
$RM -f version.[oc]
v=`$CAT version` u=${USER-root} d=`pwd` h=`$_HOSTNAME` t=`$DATE`
$SED -e "s|%WHEN%|${t}:/|" -e "s|%WHOANDWHERE%|${u}@${h}:${d}|" \
	< $1 >version.c
$ECHO `$EXPR ${v} + 1` > version
