#!/bin/ksh
#  @(#)52        1.1  src/packages/X11/apps/util/usr/X11.apps.util.post_i.sh, pkgx, pkg411, GOLD410 6/3/94 15:44:58
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.apps.util (usr) post installation configuration
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
#  Remove obsolete utility files not handled by rm_inv
#----------------------------------------------------------------
rm -f /usr/lpp/X11/Xamples/util/cpp/xcpp

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
