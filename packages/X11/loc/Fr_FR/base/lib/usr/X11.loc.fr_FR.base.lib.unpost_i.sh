#!/bin/ksh
#  @(#)28        1.3  src/packages/X11/loc/fr_FR/base/lib/usr/X11.loc.fr_FR.base.lib.unpost_i.sh, pkgx, pkg411, 9437A411a 9/12/94 15:40:30
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.loc.fr_FR.base.lib (usr) unpost installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#  Remove Customiziing Tool from Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/custom -l fr_FR -u > /dev/null 2>&1

#----------------------------------------------------------------
#  Remove X examples Tool from Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/Xamples -l fr_FR -u > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
