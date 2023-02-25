#!/bin/ksh
# @(#)20	1.2  src/packages/X11/fnt/coreX/usr/X11.fnt.coreX.post_u.sh, pkgx, pkg41B, 412_41B_sync 1/16/95 17:04:32
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

for FONTSUBDIR in 100dpi 75dpi misc; do
  #----------------------------------------------------------------
  #   Create fonts.dir file for the X11 100dpi, 75dpi and misc fonts
  #----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts/$FONTSUBDIR
if [ -f $FONTDIR/fonts.dir ]; then
   rm -f $FONTDIR/fonts.dir || exit 1
fi
if [ -x /usr/lpp/X11/bin/mkfontdir ]; then
  /usr/lpp/X11/bin/mkfontdir $FONTDIR > /dev/null 2>&1 || exit 1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value

