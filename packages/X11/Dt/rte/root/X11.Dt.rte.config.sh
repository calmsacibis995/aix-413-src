# @(#)73	1.6  src/packages/X11/Dt/rte/root/X11.Dt.rte.config.sh, pkgxcde, pkg411, GOLD410 6/2/94 18:40:08
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
#
# FUNCTION: script called from install to automatically start CDE
#	    Desktop.
#
# RETURN VALUE DESCRIPTION:
#				nonzero return code if failed
#
#
#
# configure the dtspc daemon
#
/usr/dt/install/configMin -e
if [ $? != 0 ]; then
   exit 1
fi
#
# configure the runtime environment
#
/usr/dt/install/configRun -e
if [ $? != 0 ]; then
   exit 1
fi
#
# enable desktop login
#
/usr/dt/bin/dtconfig -e
if [ $? != 0 ]; then
   exit 1
fi
