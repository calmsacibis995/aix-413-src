#!/bin/bsh
# @(#)41	1.2  src/tcpip/usr/sbin/mosy/version.sh, isodelib7, tcpip410, tcpip410greg1 3/3/93 09:11:28
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
# FILE:		src/tcpip/usr/sbin/mosy/version.sh
#

: run this script through /bin/sh

: this hacks past a bug in make...
exec 3<&- 4<&- 5<&- 6<&- 7<&-

ECHO=$ODE_TOOLS/usr/bin/echo
AWK=$ODE_TOOLS/usr/bin/awk
RM=$ODE_TOOLS/usr/bin/rm
CAT=$ODE_TOOLS/usr/bin/cat
DATE=$ODE_TOOLS/usr/bin/date
SED=$ODE_TOOLS/usr/bin/sed

OFS="$IFS" IFS=:

if [ "x$1" = x ]; then ${ECHO} 'usage: version.sh module' 1>&2; exit 1; fi

#for A in rprompt hostname uname who
#do
#    for D in $PATH
#    do
#	if [ ! -f $D/$A ]; then
#	    continue
#	fi
#	case $A in
#	    rprompt)    LOCAL=`$A %h`
#			;;
#	    hostname)   LOCAL=`$A`
#			;;
#	    uname)	LOCAL=`$A -n`
#			;;
#	    who)	LOCAL=`$A am i | sed -e 's%^\(.*\)!.*$%\1%'`
#			;;
#	esac
#	break
#    done
#    if [ "x$LOCAL" != x ]; then
#	break
#    fi
#done

IFS=

if [ ! -r version.major ]; then ${ECHO} 6 > version.major; fi
if [ ! -r version.minor ]; then ${ECHO} 8 > version.minor; fi
if [ ! -r version.local ]; then ${ECHO} 0 > version.local; fi
${ECHO} `${CAT} version.major` `${CAT} version.minor` `${CAT} version.local` $1 $2 > version
${RM} -f version.c version.local

${AWK} '	{ major = $1; minor = $2; local = $3 + 1; sfw = $4; \
	  if (NF >= 5) note = $5; else note = ""; };\
END	{ printf "char *%sversion = \"%s%s %d.%d", sfw, sfw, note, major, minor; \
	  printf "%d\n", local > "version.local"; }' < version
${ECHO} ' of '`${DATE}`'";'

${RM} -f version
