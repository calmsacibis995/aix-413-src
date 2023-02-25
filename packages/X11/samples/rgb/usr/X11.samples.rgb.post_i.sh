#!/bin/ksh
#  @(#)62        1.1  src/packages/X11/samples/rgb/usr/X11.samples.rgb.post_i.sh, pkgx, pkg411, GOLD410 6/8/94 08:27:30
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.rgb (usr) post installation configuration
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
#   Add customer courtesy links for sample showrgb client
#     Add symbolic link to /usr/lpp/X11/bin
#     Add symbolic link to /usr/lpp/X11/Xamples/man
#----------------------------------------------------------------
rm -f /usr/lpp/X11/bin/showrgb
ln -sf /usr/lpp/X11/Xamples/bin/showrgb /usr/lpp/X11/bin/showrgb
rm -f /usr/lpp/X11/Xamples/man/showrgb.man
ln -sf /usr/lpp/X11/Xamples/rgb/showrgb.man /usr/lpp/X11/Xamples/man/showrgb.man

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
