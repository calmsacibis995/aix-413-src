#!/bin/ksh
# @(#)83	1.3  src/packages/bos/diag/rte/root/bos.diag.rte.config.sh, pkgdiag, pkg411, GOLD410 4/11/94 09:39:43
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
# Configuration Script for AIX on the RS/6000
#
# Usage:
#	bos.diag.config
#
# Output:
#	0/1 exit code
#
# This script will perform the necessary configurations to enable
# periodic diagnostics.
#
#########################################################################
INUUMSGID=140

/usr/sbin/lsitab diagd >/dev/null 2>&1
if [ $? -eq 1 ]
then
# entry does not exist in /etc/inittab
    /usr/sbin/mkitab "diagd:2:once:/usr/lpp/diagnostics/bin/diagd >/dev/console 2>&1"
    if [ $? -ne 0 ]
    then
	/usr/bin/dspmsg inuumsg.cat -s 1 $INUUMSGID \
	'WARNING: mkitab failed! Unable to add \"diagd\" entry to /etc/inittab.\n'
	exit 1
    fi
fi
# Now start the diagnostic daemon

/usr/lpp/diagnostics/bin/diagd

exit 0


