#!/bin/ksh
# @(#)29        1.3  src/packages/X11/loc/zh_CN/base/lib/usr/X11.loc.zh_CN.base.lib.post_i.sh, ils-zh_CN, pkg41J, 9513A_all 3/24/95 15:04:51
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.loc.zh_CN.base.lib (usr) post installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#----------------------------------------------------------------
#  Integrate Customiziing Tool into Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/custom -l zh_CN > /dev/null 2>&1

#----------------------------------------------------------------
#  Integrate X Examples into Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/Xamples -l zh_CN > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
