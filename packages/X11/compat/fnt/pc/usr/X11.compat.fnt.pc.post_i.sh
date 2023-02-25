#!/bin/ksh
#  @(#)60        1.3  src/packages/X11/compat/fnt/pc/usr/X11.compat.fnt.pc.post_i.sh, pkgx, pkg411, GOLD410 3/30/94 15:17:03
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.compat.fnt.pc (usr) post installation configuration
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
#   Create fonts.dir file for the X11 ibm850 fonts
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts/ibm850
if [ -f $FONTDIR/fonts.dir ]; then
  rm -f $FONTDIR/fonts.dir || exit 1
fi
if [ -x /usr/lpp/X11/bin/mkfontdir ]; then
  /usr/lpp/X11/bin/mkfontdir $FONTDIR > /dev/null 2>&1 || exit 1
fi

#----------------------------------------------------------------
#   Add entries to X11 ibm850 fonts.alias
#----------------------------------------------------------------
if [ ! -f $FONTDIR/fonts.alias ]; then
  echo "FILE_NAMES_ALIASES" > $FONTDIR/fonts.alias || exit 1
fi
cat $FONTDIR/fonts.alias.ibm850 >> $FONTDIR/fonts.alias || exit 1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
