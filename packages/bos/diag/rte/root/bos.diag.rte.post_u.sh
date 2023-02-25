#!/bin/ksh
# @(#)51	1.1  src/packages/bos/diag/rte/root/bos.diag.rte.post_u.sh, pkgdiag, pkg41J, 9513A_all 3/27/95 14:43:29
#
#   COMPONENT_NAME: PKGDIAG
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#########################################################################
# Post-update Script for AIX on the RS/6000
#
# Usage:
#	bos.diag.post_u
#
# Output:
#	0/1 exit code
#
# This script will perform the necessary configurations to enable
# periodic diagnostics.
#
#########################################################################

# Start the diagnostic daemon if it's not already running.

LOCK_FILE="/etc/lpp/diagnostics/data/diagd.lck"

if [ -f $LOCK_FILE ]
then
	pid=`od -t d4 -w -A n $LOCK_FILE`
	/usr/bin/ps $pid >/dev/null 2>&1
	if [ $? -eq 1 ]
	then
		/usr/lpp/diagnostics/bin/diagd
	fi
fi
exit 0

