# @(#)75	1.6  src/packages/X11/Dt/rte/root/X11.Dt.rte.unconfig.sh, pkgxcde, pkg41J, 9519B_all 4/11/95 11:28:46
#
#   COMPONENT_NAME: pkgxcde
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
# FUNCTION: script called from deinstal to automatically stop CDE
#	    Desktop.
#
# RETURN VALUE DESCRIPTION:
#				nonzero return code if failed
#
#
# unconfigure the dtspc daemon
#
/usr/dt/install/configMin -d
#
# unconfigure the desktop
# 
/usr/dt/install/configRun -d
#
# unconfigure desktop login
#
/usr/dt/bin/dtconfig -d
#
# remove the desktop subsystem
#
rmssys -s dtsrc
#
# remove the /var/dt directories
#
rm -rf /var/dt > /dev/null 2>&1

