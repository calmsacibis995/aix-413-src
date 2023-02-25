#!/bin/ksh
#  @(#)52        1.1  src/packages/X11/apps/custom/usr/X11.apps.custom.unpost_i.sh, pkgx, pkg411, GOLD410 6/27/94 15:33:20
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.apps.custom (usr) unpost installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#  Remove Customizing Tool from Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/custom -l C -u

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
