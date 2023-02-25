#!/bin/ksh
#  @(#)64        1.3  src/packages/X11/fnt/coreX/usr/X11.fnt.coreX.post_i.sh, pkgx, pkg411, GOLD410 3/30/94 15:17:13
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
  #   Set up 100dpi, 75dpi and misc fonts.alias
  #----------------------------------------------------------------
  if [ ! -f $FONTDIR/fonts.alias ]; then
    echo "FILE_NAMES_ALIASES" > $FONTDIR/fonts.alias || exit 1
  fi
  cat $FONTDIR/fonts.alias.$FONTSUBDIR >> $FONTDIR/fonts.alias || exit 1
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
