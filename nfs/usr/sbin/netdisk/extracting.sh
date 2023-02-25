#!/bin/ksh
# 
# COMPONENT_NAME: (CMDNFS) Network File System Commands
# 
# FUNCTIONS: 
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
#  extract tapedev fsf bs keywords tapeserver 
#
#  NOTE!  This script assumes that the underlying implementations of
#  tar and dd agree about what size a block is, and that it is 512 bytes
#  long.  If this is not the case, then this script and setup_exec will
#  have to be modified.
#
TAPECMD=tctl
ALT_TAPECMD=mt

cmdname=$0

case $# in
4 | 5)
	;;
*)
	echo Usage: $cmdname tapedev fsf bs keywords tapeserver 
	exit 1
esac

tapedev=$1
case "$tapedev" in
cd*)
	CDFS="yes"
	keywords=$2
	cdfs_pathname=$3
	where=$4
	arch=$5
	;;
*)	
	fsf=$2
	bs=$3
	keywords=$4
	tapeserver=$5
	;;
esac

EXTRACTMSG="Extracting \"${keywords}\" files from \"${tapedev}\" release media."

if [ -z "$CDFS" ] ; then
case "${tapeserver}" in
"")
	REMOTE=""
	;;
*)
        REMOTE="rsh ${tapeserver} -n"
	;;
esac

# Do this in case mt is the tape control program on the remote host
case "${REMOTE}" in
"")
	$TAPECMD -f ${tapedev} fsf ${fsf}
	;;
*)
	${REMOTE} "$TAPECMD -f ${tapedev} fsf ${fsf} || $ALT_TAPECMD -f ${tapedev} fsf ${fsf}" > /dev/null 2>&1
	;;
esac
fi

echo; echo "${EXTRACTMSG}" 
case "${REMOTE}" in
"")
	if [ -z "$CDFS" ] ; then
		tar -xpb ${bs} -f ${tapedev}
	else
		case "$where" in
		"impl")
			pathname="$cdfs_pathname/export/exec/kvm/$arch*/$keywords"
			;;
		"appl")
			case "$arch" in
			sun4*)
				pathname="$cdfs_pathname/export/exec/sun4*/$keywords"
				;;
			*)
				pathname="$cdfs_pathname/export/exec/sun3*/$keywords"
			esac
			;;
		"share")
			pathname=`ls $cdfs_pathname/export/share/sunos*([_0-9])/$keywords`
			;;
		"/")
			pathname=`ls $cdfs_pathname/export/exec/proto_root_sunos*([_0-9])`
			;;
		esac
		tar -xpf ${pathname}
	fi
	;;
*)
	${REMOTE} dd if=${tapedev} bs=${bs}b 2>/dev/null \
	  | tar xpBf - 2>/dev/null
	;;
esac
exit 0
