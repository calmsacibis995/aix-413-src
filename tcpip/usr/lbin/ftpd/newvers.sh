#!/bin/sh 
# @(#)16        1.5  src/tcpip/usr/lbin/ftpd/newvers.sh, tcpfilexfr, tcpip411, GOLD410 2/22/93 19:27:39
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
# Copyright (c) 1983 The Regents of the University of California.
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
#	newvers.sh	5.3 (Berkeley) 10/31/88
#
ECHO=$ODE_TOOLS/usr/bin/echo
AWK=$ODE_TOOLS/usr/bin/awk
DATE=$ODE_TOOLS/usr/bin/date
TOUCH=$ODE_TOOLS/usr/bin/touch

if [ ! -r version ] ; then $ECHO "0" > version ; fi
$TOUCH version
$AWK '	{	version = $1 + 1; }
END	{	printf "char version[] = \"Version 4.%d ", version > "vers.c";\
		printf "%d\n", version > "version"; }' < version
$ECHO `$DATE`'";' >> vers.c
