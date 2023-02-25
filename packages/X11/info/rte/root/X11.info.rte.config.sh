#!/usr/bin/bsh
# @(#)74	1.6  src/packages/X11/info/rte/root/X11.info.rte.config.sh, pkginfo, pkg411, GOLD410  6/27/94  17:52:22
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
# NAME: X11.info.rte.config 
#                                                                    
# FUNCTION: script called from install to do special infod configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
 
#============================================================#
#
# This adds the startrc infod line to inittab, notifies
# SRC of the infod daemon, and starts infod.
#
#============================================================#
#
# Constants
#
# the pathname of the inittab file
#
    inittab=/etc/inittab
#
# the error message numbers for inuumsg
#
    eprog=36  # "program %s failed"
 
#============================================================#
#
# Do an error message and exit
#
# $1:  program name that caused failure (to supply to the error message)

error()
{
    inuumsg $eprog "$1"
    exit 1
}

#
# Add the startsrc infod entry in the inittab file
#
    rmitab infod 1>/dev/null 2>&1

    {
      mkitab "infod:2:once:startsrc -s infod"
    } || error mkitab
#
# Start infod if this is not a client.
# Assumes $INUCLIENTS will be non-zero length if installing on client.
#
    if [ "$INUCLIENTS" = "" ]
    then
      # Remove the old infod.  infod should already be stopped.
      rmssys -s infod 1>/dev/null 2>&1

      # Add the src stuff
      {
        mkssys -s infod -p /usr/lpp/info/bin/infod -u 0 -S -n 9 -f 9 -d -R -G infod
      } || error mkssys

      # Start the new infod.
      startsrc -s infod
    fi
#
# Make symlink if CDE desktop is installed, else make symlink to make the
# appmanager directory and make a symlink.
#
   if [ -d /usr/dt/appconfig/appmanager/C/Information ]
   then
	ln -s /usr/lpp/info/dt/appconfig/InfoExp /usr/dt/appconfig/appmanager/C/Information/InfoExp
   else
	mkdir -p /usr/lpp/info/dt/appconfig/appmanager/C
	ln -s /usr/lpp/info/dt/appconfig/InfoExp /usr/lpp/info/dt/appconfig/appmanager/C/InfoExp
   fi 
#
# Integrate infoExplorer to CDE desktop
#
   {
      /etc/dtappintegrate -s /usr/lpp/info
   } || error dtappintegrate
#
# Exit normally
#
exit 0
