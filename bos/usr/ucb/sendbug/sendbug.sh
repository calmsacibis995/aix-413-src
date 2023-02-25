#!/bin/bsh 
# @(#)11	1.3  src/bos/usr/ucb/sendbug/sendbug.sh, cmdmailx, bos411, 9428A410j 10/31/90 13:45:44
# COMPONENT_NAME: CMDMH sendbug
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
# Copyright (c) 1983 Regents of the University of California.
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
#
# Create a bug report and mail to '4bsd-bugs'.

TEMP=/tmp/bug$$
FORMAT=/usr/lib/bugformat

: ${BUGADDR=postmaster}
: ${EDITOR=/usr/bin/vi}

trap '/bin/rm -f $TEMP ; exit 1' 1 2 3 13 15

/bin/cp $FORMAT $TEMP
chmod 644 $TEMP
if $EDITOR $TEMP
then
	if cmp -s $FORMAT $TEMP
	then
		echo "File not changed, no bug report submitted."
		exit
	fi
	case "$#" in
	0) /usr/lib/sendmail -t -oi $BUGADDR  < $TEMP ;;
	*) /usr/lib/sendmail -t -oi "$@" < $TEMP ;;
	esac
fi

/bin/rm -f $TEMP
