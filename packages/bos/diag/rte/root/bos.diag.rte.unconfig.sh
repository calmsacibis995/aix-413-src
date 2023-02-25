#!/bin/ksh
# @(#)84	1.4  src/packages/bos/diag/rte/root/bos.diag.rte.unconfig.sh, pkgdiag, pkg411, GOLD410 4/11/94 09:40:05
#
#   COMPONENT_NAME: PKGDIAG
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#########################################################################
#
# Un-configuration Script for AIX on the RS/6000
#
# Usage:
#	bos.diag.unconfig
#
# Output:
#	0/1 exit code
#
#########################################################################


# Remove the "diagd" entry from the inittab file first.  If we can't
# do that, complain, but continue.

INUUMSGID=141
LOCK_FILE="/etc/lpp/diagnostics/data/diagd.lck"

# First kill the daemon if it is running

if [ -f $LOCK_FILE ]
then
	od -t d4 -w -A n $LOCK_FILE | xargs kill -9
fi

# Remove entry from inittab file
/usr/sbin/rmitab diagd > /dev/null 2>&1
if [ $? -ne 0 ]
then
	/usr/bin/dspmsg inuumsg.cat -s 1 $INUUMSGID \
	  'WARNING: rmitab failed! Unable to remove \"diagd\" entry from /etc/inittab.\n'
fi
exit 0
