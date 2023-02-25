#!/bin/bsh
# @(#)11	1.3  src/packages/uimx/rte/usr/uimx.rte.unconfig.sh, pkguimx, pkg411, 9438C411b 9/25/94 14:46:24
 
#
#   COMPONENT_NAME: pkguimx
#
#   FUNCTIONS: deinstallation configuration
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
# NAME: uimx.rte.unconfig.sh
#                                                                   
# FUNCTION: script called from installp to cleanup an uxserverd installation
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
# 

    eprog=36  # "program %s failed"

# location of license file
    UIMXLICENSEFILE=/usr/uimx2.8/Uimx2_8.lic


#============================================================#
#
# Do an error message and exit
#
# $1:  program name that caused failure (to supply to the error message)

error()
{
    inuumsg $eprog "$1"
    exit 0
}

#============================================================#
# Stop running uxserverd daemon
#
    {
      lssrc -s uxserverd | /usr/bin/grep -q active
    } && stopsrc -s uxserverd

#============================================================#
# unintegrate uimx application from the desktop
#
    {
      /usr/sbin/dtappintegrate -s /usr/uimx2.8 -u
    } || error dtappintegrate

#============================================================#
#
# Remove the SRC entry
#
    {
      lssrc -s uxserverd 1>/dev/null
    } && {
           {
             rmssys -s uxserverd
           } || error rmssys
         }

#============================================================#
#
# Remove the inittab entry
#
    {
      lsitab uxserverd 1>/dev/null
    } && {
           {
             rmitab uxserverd
           } || error rmitab
         }

#============================================================#
#
# Remove license file if it exists
#
    if [ -f $UIMXLICENSEFILE ]
    then
       /usr/bin/rm -f $UIMXLICENSEFILE
    fi

#
# Exit normally
#
    exit 0

