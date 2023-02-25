#!/bin/bsh
# @(#)77    1.5  src/bos/usr/sbin/sa/sa1.sh, cmdstat, bos411, 9428A410j 5/26/93 20:08:07
#
# COMPONENT_NAME: (CMDSTAT) status
#
# FUNCTIONS:
#
# ORIGINS: 3, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# Copyright 1976, Bell Telephone Laboratories, Inc.
#

DATE=`date +%d`
ENDIR=/usr/lib/sa
DFILE=/var/adm/sa/sa$DATE
cd $ENDIR
if [ $# = 0 ]
then
	$ENDIR/sadc 1 1 $DFILE
else
	$ENDIR/sadc $* $DFILE
fi
