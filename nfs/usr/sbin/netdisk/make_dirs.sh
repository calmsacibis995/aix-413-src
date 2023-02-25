#!/bin/bsh
# @(#)65	1.4  src/nfs/usr/sbin/netdisk/make_dirs.sh, cmdnfs, nfs411, GOLD410 2/2/94 15:40:43
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
#  Copyright 1987 Sun Microsystems, Inc.
#
#  Shell script to make specified directory and
#  its parents, if neccessary.
#
#  usage: make_dirs <dir> [mode]
#
#

if [ $# -eq 1 ] 
then
	MODE=""
elif [ $# -eq 2 ] 
then
	MODE=$2
else
	echo "Usage: $0 dirname [mode]"
	exit 1
fi


DIR=$1
SUBDIRS=`echo $DIR | /usr/bin/tr '/' ' '`

for subdirectory in $SUBDIRS
do
	DIRECTORY="$DIRECTORY/$subdirectory"
	if [ ! -d $DIRECTORY ] 
	then
		mkdir $DIRECTORY
		if [ -n "$MODE" ]
		then
			chmod $MODE $DIRECTORY
		fi
	fi
done



