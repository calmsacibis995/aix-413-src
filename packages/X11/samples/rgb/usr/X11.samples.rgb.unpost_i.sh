#!/bin/ksh
#  @(#)63        1.1  src/packages/X11/samples/rgb/usr/X11.samples.rgb.unpost_i.sh, pkgx, pkg411, GOLD410 6/8/94 08:27:35
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.rgb (usr) unpost installation configuration
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
#   Remove customer courtesy links for sample clients
#     Remove symbolic link to /usr/lpp/X11/bin
#     Remove symbolic link to /usr/lpp/X11/Xamples/man
#----------------------------------------------------------------
rm -f /usr/lpp/X11/bin/showrgb
rm -f /usr/lpp/X11/Xamples/man/showrgb.man

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
