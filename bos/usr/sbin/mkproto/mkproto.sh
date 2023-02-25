#!/usr/bin/bsh
# @(#)51 1.9  src/bos/usr/sbin/mkproto/mkproto.sh, cmdfiles, bos411, 9428A410j 9/1/93 16:47:09
#
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
#
# FUNCTIONS: mkproto
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# mkproto special proto
#
#   BSD interface to mkfs
#
# PTM 35193: Redirected the Error Message to stderr.

if [ $# != 2 -o "$1" = "" -o "$2" = "" ] ; then
	if msg=`dspmsg -s 1 mkproto.cat 1 "Usage: mkproto special proto"`
	then :
	else msg="Usage: mkproto special proto" 
	fi
	echo $msg >&2
	exit 1
fi

exec mkfs -p "$2" $1
