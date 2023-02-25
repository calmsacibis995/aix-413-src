#!/bin/ksh
#  @(#)69        1.3  src/packages/X11/fnt/iso1/usr/X11.fnt.iso1.post_i.sh, pkgx, pkg411, GOLD410 5/16/94 09:21:17
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.iso1 (usr) post installation configuration
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
#   Create fonts.dir file for the X11 fonts
#----------------------------------------------------------------
fontdirlist="/usr/lpp/X11/lib/X11/fonts	\
			/usr/lpp/X11/lib/X11/fonts/100dpi"

for dir in $fontdirlist; do
  if [ -f $dir/fonts.dir ]; then
    rm -f $dir/fonts.dir || exit 1
  fi
  if [ -x /usr/lpp/X11/bin/mkfontdir ]; then
    /usr/lpp/X11/bin/mkfontdir $dir > /dev/null 2>&1 || exit 1
  fi
done

#----------------------------------------------------------------
#   Add iso1 entries to X11 fonts.alias
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts
if [ ! -f $FONTDIR/fonts.alias ]; then
  echo "FILE_NAMES_ALIASES" > $FONTDIR/fonts.alias || exit 1
fi
cat $FONTDIR/fonts.alias.iso1 >> $FONTDIR/fonts.alias || exit 1

#----------------------------------------------------------------
#   Set up 100dpi fonts.alias
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts/100dpi
if [ ! -f $FONTDIR/fonts.alias ]; then
  echo "FILE_NAMES_ALIASES" > $FONTDIR/fonts.alias || exit 1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
