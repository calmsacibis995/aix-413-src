#!/bin/bsh
# @(#)78      1.7  src/nfs/usr/sbin/netdisk/verify_tapevol_arch.sh, cmdnfs, nfs411, GOLD410 2/3/94 16:39:43
# 
# COMPONENT_NAME: (CMDNFS) Network File System Commands
# 
# FUNCTIONS: tape 
#
# ORIGINS: 24 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#       Copyright (c) 1987 by Sun Microsystems, Inc.
#
# verify_tapevol_arch arch tapen tapedev tapeserver
#

HOME=/; export HOME
PATH=/bin:/usr/bin:/etc:/usr/sbin:/usr/ucb:/usr/lpp/nfs/sun_diskless
CMDNAME=$0
case $# in
3 | 4)
	;;
*)
        echo Usage: $CMDNAME arch tapen tapedev tapeserver
        exit 1 ;;
esac

ARCH=$1
TAPEN=$2 
TAPEDEV=$3
TAPESERVER=$4
TAPECMD=tctl
ALT_TAPECMD=mt
ARCH_TAPE=""
TAPE_NUM=""
INSTALLPATH="/tmp"

case "${TAPESERVER}" in
"")
	REMOTE=""
	;;
*)
        REMOTE="rsh ${TAPESERVER} -n"
	;;
esac

while true; do
	while true; do
		case "${REMOTE}" in
		"")
			STRING=`$TAPECMD -f ${TAPEDEV} rewind 2>&1`
			STRING=`$TAPECMD -f ${TAPEDEV} rewind 2>&1`
			;;
		*)
			STRING=`${REMOTE} $TAPECMD -f ${TAPEDEV} rewind 2>&1`
			STRING=`${REMOTE} $TAPECMD -f ${TAPEDEV} rewind 2>&1`
			if [ -n "${STRING}" ] ; then
				STRING=`${REMOTE} ${ALT_TAPECMD} -f ${TAPEDEV} rewind 2>&1`
			fi
		esac
		case "${STRING}" in
		"")
			case "${REMOTE}" in
			"")
				$TAPECMD -f ${TAPEDEV} fsf 1 >/dev/null 2>&1
				;;
			*)
	   			${REMOTE} "$TAPECMD -f ${TAPEDEV} fsf 1 || ${ALT_TAPECMD} -f ${TAPEDEV} fsf 1 " > /dev/null 2>&1
				;;
			esac
			break 
			;;
		*)
	   		echo "${STRING}"
	   		echo "Tape drive ${TAPEDEV} not ready."
                	echo "
Load release tape #${TAPEN} for architecture ${ARCH} and hit <RETURN>: \c"
                	read x
			;;
		esac
	done
	#
	# get TOC from release tape (dd format)
	#
	rm -f ${INSTALLPATH}/TOC
	case "${REMOTE}" in
	"")
		dd if=${TAPEDEV} bs=200k 2>/dev/null \
		  | xdrtoc > ${INSTALLPATH}/TOC
		;;
	*)
		${REMOTE} dd if=${TAPEDEV} bs=200k 2>/dev/null \
		  | xdrtoc > ${INSTALLPATH}/TOC
		;;
	esac
	ARCH_TAPE=`awk '/^ARCH/ { print $2 }' < ${INSTALLPATH}/TOC`
	TAPE_NUM=`awk '/^VOLUME/ { print $2 }' < ${INSTALLPATH}/TOC`
	case "${ARCH_TAPE}" in
	"${ARCH}" )
		case "${TAPE_NUM}" in
		"${TAPEN}" )
			break
			;;
		*)
			echo "Tape loaded is #${TAPE_NUM}"
			;;
		esac ;;
	*)
		echo "Tape is wrong architecture (${ARCH_TAPE})"
		;;
	esac
	echo "
Load release tape #${TAPEN} for architecture ${ARCH} and hit <RETURN>: \c"
	read x
done
case "${REMOTE}" in
"")
	STRING=`$TAPECMD -f ${TAPEDEV} rewind 2>&1`
	STRING=`$TAPECMD -f ${TAPEDEV} rewind 2>&1`
	;;
*)
	${REMOTE} "$TAPECMD -f ${TAPEDEV} rewind || $ALT_TAPECMD -f ${TAPEDEV} rewind" > /dev/null 2>&1
	${REMOTE} "$TAPECMD -f ${TAPEDEV} rewind || $ALT_TAPECMD -f ${TAPEDEV} rewind" > /dev/null 2>&1
esac

sync; sync
