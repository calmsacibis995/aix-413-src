#! /usr/bin/bsh
# @(#)75	1.5  src/packages/X11/info/rte/root/X11.info.rte.unconfig.sh, pkginfo, pkg411, GOLD410  6/27/94  17:52:25
#
# COMPONENT_NAME: pkginfo
#
# FUNCTIONS: X11.info.rte installation configuration
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: X11.info.rte.unconfig
#                                                                   
# FUNCTION: script called from installp to cleanup an infod installation
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
# 

    inittab=/etc/inittab

#============================================================#
#
# Remove the inittab entry
#
    rmitab infod 1>/dev/null 2>&1

#============================================================#
#
# Stop infod if this is not a client.
# Assumes $INUCLIENTS will be non-zero length if installing on client.
#
    if [ "$INUCLIENTS" = "" ]
    then
      stopsrc -s infod

      # Remove the SRC entry
      rmssys -s infod 1>/dev/null 2>&1
    fi
#
#  undo dtappintegrate
#
    /etc/dtappintegrate -s /usr/lpp/info -u 1>/dev/null 2>&1
#
# Remove the symlink to CDE desktop
#
    if [ -f /usr/dt/appconfig/appmanager/C/Information/InfoExp ]
    then
	rm /usr/dt/appconfig/appmanager/C/Information/InfoExp
    fi
#
# Remove /usr/lpp/info/dt/appconfig/appmanager directory
#
    if [ -f /usr/lpp/info/dt/appconfig/appmanager/C/InfoExp ]
    then
	rm -r /usr/lpp/info/dt/appconfig/appmanager
    fi
# Exit normally
exit 0
