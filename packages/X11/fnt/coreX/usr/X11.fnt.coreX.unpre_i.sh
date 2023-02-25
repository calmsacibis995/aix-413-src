#!/bin/ksh
#  @(#)65        1.3  src/packages/X11/fnt/coreX/usr/X11.fnt.coreX.unpre_i.sh, pkgx, pkg411, GOLD410 4/6/94 17:42:50
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.coreX (usr) unpre installation configuration
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
for FONTSUBDIR in 100dpi 75dpi misc; do
  #----------------------------------------------------------------
  #   Create fonts.dir file for the X11 fonts
  #----------------------------------------------------------------
  FONTDIR=/usr/lpp/X11/lib/X11/fonts/$FONTSUBDIR
  if [ -f $FONTDIR/fonts.dir ]; then
    rm -f $FONTDIR/fonts.dir || exit 1
  fi
  if [ -x /usr/lpp/X11/mkfontdir ]; then
    /usr/lpp/X11/bin/mkfontdir $FONTDIR > /dev/null 2>&1
  fi
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
