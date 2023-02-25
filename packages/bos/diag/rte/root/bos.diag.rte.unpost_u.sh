#!/bin/ksh
# @(#)53	1.1  src/packages/bos/diag/rte/root/bos.diag.rte.unpost_u.sh, pkgdiag, pkg41J, 9513A_all 3/27/95 14:43:54
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
# Undo Post-update Script for AIX on the RS/6000
#
# Usage:
#	bos.diag.unpost_u
#
# Output:
#	0/1 exit code
#
# This script will perform the necessary cleanup to kill the 
# periodic diagnostics.
#
#########################################################################

# Kill the "diagd" daemon.

LOCK_FILE="/etc/lpp/diagnostics/data/diagd.lck"

if [ -f $LOCK_FILE ]
then
	od -t d4 -w -A n $LOCK_FILE | xargs kill -9
fi

exit 0


