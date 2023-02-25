#!/bin/ksh
#  @(#)00        1.2  src/packages/X11/compat/fnt/oldX/usr/X11.compat.fnt.oldX.post_i.sh, pkgx, pkg411, GOLD410 3/30/94 15:16:39
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.coreX (usr) post installation configuration
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
