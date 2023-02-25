#!/bin/ksh
#  @(#)30        1.3  src/packages/X11/loc/ja_JP/base/lib/usr/X11.loc.ja_JP.base.lib.unpost_i.sh, pkgx, pkg411, 9437A411a 9/12/94 15:40:41
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.loc.ja_JP.base.lib (usr) unpost installation configuration
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
/etc/dtappintegrate -s /usr/lpp/X11/custom -l ja_JP -u > /dev/null 2>&1

#----------------------------------------------------------------
#  Remove X examples from Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/Xamples -l ja_JP -u > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
