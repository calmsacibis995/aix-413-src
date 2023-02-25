#! /bin/ksh
# @(#)40	1.2  src/tcpip/usr/sbin/mosy/extract.sh, isodelib7, tcpip410, tcpip410greg1 3/3/93 09:10:57
#
# COMPONENT_NAME:  (ISODELIB7) ISODE Libraries, Release 7
#
# FUNCTIONS: none
#
# ORIGINS: 27, 60
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# FILE:		src/tcpip/usr/sbin/mosy/extract.sh
#

: run this script through /bin/sh

ECHO=$ODE_TOOLS/usr/bin/echo
RM=$ODE_TOOLS/usr/bin/rm
SED=$ODE_TOOLS/usr/bin/sed

# A script to pre-process files.
# a bit like ifdef in C compiler - but simpler.
# The idea is you put lines like
# %BEGIN(DEF)%
# %END(DEF)%
# in your file, this script will keep things enclosed in these
# whilst deleting all others.

case $# in
	0)	${ECHO} "$0: Usage: $0 Definition [Defs...]" 1>&2; exit 1;;
esac

tfile=/tmp/extr.$$
trap "${RM} -f $tfile;exit" 1 2 15

${ECHO}	'/%WARNING%/s//This file produced automatically, do not edit!/' > $tfile
for i
do
	${ECHO} "/%BEGIN($i)%/d"
	${ECHO} "/%END($i)%/d"
done >> $tfile

${ECHO} "/%BEGIN(.*)%/,/%END(.*)%/d" >> $tfile
${SED} -f $tfile
${RM} -f $tfile
