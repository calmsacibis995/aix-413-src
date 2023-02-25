#!/bin/ksh
# @(#)54	1.1  src/packages/bos/diag/rte/root/bos.diag.rte.unpre_u.sh, pkgdiag, pkg41J, 9513A_all 3/27/95 14:44:04
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
#
# Undo Pre-update Script for AIX on the RS/6000
#
# Usage:
#	bos.diag.unpre_u
#
# Output:
#	0/1 exit code
#
#########################################################################

LOCK_FILE="/etc/lpp/diagnostics/data/diagd.lck"

# Just start the diagnostic daemon if it's not running.

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


