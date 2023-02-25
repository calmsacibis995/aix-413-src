#!/bin/ksh
# @(#)33	1.2  src/packages/X11/compat/fnt/oldX/usr/X11.compat.fnt.oldX.post_u.sh, pkgx, pkg41B, 412_41B_sync 1/16/95 17:05:14
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

for FONTSUBDIR in bmug info-mac oldx10 oldx11; do
  #----------------------------------------------------------------
  #   Create fonts.dir file for the X11 100dpi, 75dpi and misc fonts
  #----------------------------------------------------------------
  FONTDIR=/usr/lpp/X11/lib/X11/fonts/$FONTSUBDIR
  /usr/lpp/X11/bin/mkfontdir $FONTDIR > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    exit 1
  fi
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
