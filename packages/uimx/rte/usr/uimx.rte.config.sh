#!/bin/bsh
# @(#)27	1.2  src/packages/uimx/rte/usr/uimx.rte.config.sh, pkguimx, pkg411, 9438C411b 9/25/94 14:46:22
 
#
#   COMPONENT_NAME: pkguimx
#
#   FUNCTIONS: installation configuration
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
# NAME: uimx.rte.config.sh
#                                                                    
# FUNCTION: script called from install to do special uxserverd configuration
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#  
 
#============================================================#
#
# This adds the startrc uxserverd line to inittab and notifies
# SRC of the uxserverd daemon
#
#============================================================#
#
# Constants
#
# the pathname of the inittab file
#
    inittab=/etc/inittab

# the error message numbers for inumsg
#
    eprog=36  # "program %s failed"

# environment variable for uimx commands (ie. uxkeygen)
    UIMXDIR=/usr/uimx2.8
    export UIMXDIR
 
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

# Stop running uxserverd daemon
#
    {
      lssrc -s uxserverd 1>/dev/null
    } && stopsrc -s uxserverd

# Add the src stuff
#
    {
      lssrc -s uxserverd 1>/dev/null
    } && rmssys -s uxserverd

    {
      mkssys -s uxserverd -p /usr/uimx2.8/bin/uxserverd -u 0 -S -n 9 -f 9 -d -R -G uxserverd
    } || error mkssys

#
# Add the startrc uxserverd entry in the inittab file
#
    {
      lsitab uxserverd 1>/dev/null
    } && rmitab uxserverd

    {
      mkitab "uxserverd:2:once:startsrc -s uxserverd"
    } || error mkitab

#
# add some licenses
#
    {
	UIMXHOST=`/bin/hostname`
	case "${UIMXHOST-NULL}" in
	    "" | NULL)
		UIMXHOST="localhost"
		;;
	    *)
		;;
	esac
	/usr/uimx2.8/bin/uxkeygen ${UIMXHOST} `/usr/bin/uname -m` 1601 2 Uimx2_8-ibmr2 0000-00-00
    } || error uxkeygen

#
# integrate uimx application with the desktop
#
    {
      /usr/sbin/dtappintegrate -s /usr/uimx2.8
    } || error dtappintegrate

#
# kick off the daemon
#
    {
      /bin/startsrc -s uxserverd
    } || error startsrc

#
# Exit normally
#
    exit 0

