#!/bin/bsh
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
# fix_bootparams : script written to add/delete a client entries from 
#		   /etc/bootparams.
#
HOME=/; export HOME
PATH=/bin:/usr/bin:/etc:/usr/sbin:/usr/ucb:/usr/lpp/nfs/sun_diskless
 
CMDNAME=$0

BOOTPARAMS="/etc/bootparams"
 
USAGE="usage: ${CMDNAME} op name rootpath swappath dumppath
where:
        op          = \"add\" or \"remove\"
        name        = name of the client machine
	rootpath    = pathname of nfsroot (for \"add\")
	swappath    = pathname of nfsswap (for \"add\")
	dumppath    = pathname of nfsdump (for \"add\") or \"none\"
"

#
# Verify number of arguments
#
if [ $# -lt 2 ]; then
	echo "${CMDNAME}: incorrect number of arguments."
        echo "${USAGE}"
        exit 1
fi
#
# Assign arguments
#
SERVER=`hostname`
OP=${1}; shift
NAME=${1}; shift
#
# Start updating bootparams
#
if [ ! -f $BOOTPARAMS ]; then
	touch $BOOTPARAMS
	chmod 644 $BOOTPARAMS
fi 
case "$OP" in 
"add" )
	if [ $# -lt 3 ]; then
		echo "${CMDNAME}: incorrect number of arguments."
		echo "${USAGE}"
		exit 1
	fi
	ROOTPATH=${1}; shift
	SWAPPATH=${1}; shift
	DUMPPATH=${1}
	case $DUMPPATH in
	none)
		echo "$NAME	root=$SERVER:$ROOTPATH/$NAME \\" >> $BOOTPARAMS
		echo "	swap=$SERVER:$SWAPPATH/$NAME" >> $BOOTPARAMS
		;;
	*)
		echo "$NAME	root=$SERVER:$ROOTPATH/$NAME \\" >> $BOOTPARAMS
		echo "	swap=$SERVER:$SWAPPATH/$NAME \\" >> $BOOTPARAMS
		echo "	dump=$SERVER:$DUMPPATH/$NAME" >> $BOOTPARAMS
		;;
	esac
	break ;;
"remove" )
	# This section is a little confusing.  It seems that ed
	# will not allow the last line to be deleted from a file with
	# commands being read from stdin. It is possible that we
	# will want to do so, so we first add a scratch line at the
	# beginning of the file.  When we are done with ed, we check
	# to see if only one line remains.  If so, the file is truncated.
	# Otherwise, the first line is removed.
	# In the commands actually deleting $NAME, we are careful to
	# check that it is not embedded in another name by checking
	# the preceeding slash and trailing space, tab, backslash, or newline.
	#
	ed - $BOOTPARAMS <<END
0a
# TEMPLINE
.
g,/${NAME}[ 	\\],d
g,/${NAME}\$,d
w
q
END
	LINE=`wc -l $BOOTPARAMS | awk '{print $1}'`
	if [ "$LINE" = 1 ]; then
		> $BOOTPARAMS
	else
		ed - $BOOTPARAMS <<END
1d
w
q
END
	fi
	break ;;
* )
	echo "${CMDNAME}: invalid operation \"${OP}\""
        echo "${USAGE}"
        exit 1
esac
exit 0
