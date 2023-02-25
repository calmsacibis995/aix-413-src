#!/bin/ksh
#  @(#)70        1.5  src/packages/X11/fnt/iso2/usr/X11.fnt.iso2.post_i.sh, pkgx, pkg411, GOLD410 3/8/94 15:39:45
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.iso2 (usr) post installation configuration
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
FONTDIR=/usr/lpp/X11/lib/X11/fonts/i18n
if [ -f $FONTDIR/fonts.dir ]; then
  rm -f $FONTDIR/fonts.dir || exit 1
fi
if [ -x /usr/lpp/X11/bin/mkfontdir ]; then
  /usr/lpp/X11/bin/mkfontdir $FONTDIR > /dev/null 2>&1 || exit 1
fi

#----------------------------------------------------------------
#   Add iso2 entries to X11 i18n fonts.alias
#----------------------------------------------------------------
if [ ! -f $FONTDIR/fonts.alias ]; then
  echo "FILE_NAMES_ALIASES" > $FONTDIR/fonts.alias || exit 1
fi
cat $FONTDIR/fonts.alias.iso2 >> $FONTDIR/fonts.alias || exit 1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
