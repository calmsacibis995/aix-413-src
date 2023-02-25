# @(#)77	1.5  src/packages/X11/Dt/rte/root/X11.Dt.rte.unconfig_d.sh, pkgxcde, pkg411, GOLD410 5/13/94 19:15:22
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
# disable desktop login
#
/usr/dt/bin/dtlogin -d
#
# remove the desktop subsystem
#
rmssys -s dtsrc
