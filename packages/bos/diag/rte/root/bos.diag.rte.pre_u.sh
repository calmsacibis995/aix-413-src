#!/bin/ksh
# @(#)52	1.1  src/packages/bos/diag/rte/root/bos.diag.rte.pre_u.sh, pkgdiag, pkg41J, 9513A_all 3/27/95 14:43:41
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
# Pre-update Script for AIX on the RS/6000
#
# Usage:
#	bos.diag.pre_u
#
# Output:
#	0/1 exit code
#
#########################################################################


# First see if there is a need to even do this work

X=`odmget -q"DType=DIAGS_VERSION_4" CDiagDev`
if [ "X$X" != X ]
then
	exit 0
fi

# Kill the "diagd" daemon. It will be re-started by the post_u script.

LOCK_FILE="/etc/lpp/diagnostics/data/diagd.lck"

if [ -f $LOCK_FILE ]
then
	od -t d4 -w -A n $LOCK_FILE | xargs kill -9
fi

exit 0
