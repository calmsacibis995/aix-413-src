#!/bin/ksh
#  @(#)67        1.3  src/packages/X11/fnt/iso1/usr/X11.fnt.iso1.unpre_i.sh, pkgx, pkg411, GOLD410 5/10/94 08:53:40
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.iso1 (usr) unpre installation configuration
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
#   Create fonts.dir file for the X11 i18n fonts
#----------------------------------------------------------------
fontdirlist="/usr/lpp/X11/lib/X11/fonts	\
			/usr/lpp/X11/lib/X11/fonts/100dpi"

for dir in $fontdirlist; do
if [ -f $dir/fonts.dir ]; then
  rm -f $dir/fonts.dir || exit 1
fi
if [ -x /usr/lpp/X11/mkfontdir ]; then
  /usr/lpp/X11/bin/mkfontdir $dir > /dev/null 2>&1
fi
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
