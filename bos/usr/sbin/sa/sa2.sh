#!/bin/bsh
# @(#)78    1.5  src/bos/usr/sbin/sa/sa2.sh, cmdstat, bos411, 9428A410j 5/26/93 20:08:10
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
RPT=/var/adm/sa/sar$DATE
DFILE=/var/adm/sa/sa$DATE
ENDIR=/usr/sbin
cd $ENDIR
$ENDIR/sar $* -f $DFILE > $RPT
find /var/adm/sa \( -name 'sar*' -o -name 'sa*' \) -mtime +7 -exec rm {} \;
