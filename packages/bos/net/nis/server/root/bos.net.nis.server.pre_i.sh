#  @(#)76        1.1  src/packages/bos/net/nis/server/root/bos.net.nis.server.pre_i.sh, pkgnis, pkg411, GOLD410 1/12/93 16:16:41
#
# COMPONENT_NAME: pkgnis
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  root/bos.net.nis.server.pre_i
#
# FUNCTION: script called from instal
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

#
# This deletes the SRC subsystem definitions that we added
# with our config script ( or a previous install)

#============================================================#
#
# Delete the src stuff
#
    rmssys -s yppasswdd >/dev/null 2>&1
    rmssys -s ypupdated >/dev/null 2>&1
    rmssys -s ypserv >/dev/null 2>&1

    exit 0
