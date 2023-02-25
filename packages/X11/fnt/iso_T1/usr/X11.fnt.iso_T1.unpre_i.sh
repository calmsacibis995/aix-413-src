#!/bin/ksh
#  @(#)98        1.5  src/packages/X11/fnt/iso_T1/usr/X11.fnt.iso_T1.unpre_i.sh, pkgx, pkg411, GOLD410 4/6/94 18:06:12
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.iso_T1 (usr) unpre installation configuration
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
#   Create fonts.dir file for the X11 Type1 fonts
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts/Type1
if [ -f $FONTDIR/fonts.dir ]; then
  rm -f $FONTDIR/fonts.dir || exit 1
fi
if [ -x /usr/lpp/X11/bin/mkfontdir ]; then
  /usr/lpp/X11/bin/mkfontdir $FONTDIR > /dev/null 2>&1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
