#!/bin/ksh
#  @(#)44        1.4  src/packages/X11/fnt/iso5/usr/X11.fnt.iso5.unpost_i.sh, pkgx, pkg411, GOLD410 5/11/94 08:58:17
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.iso5 (usr) unpost installation configuration
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
#   Restore i18n fonts.alias
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts/i18n
if [ -f $FONTDIR/fonts.alias ]; then
  grep -vxf $FONTDIR/fonts.alias.iso5 $FONTDIR/fonts.alias > $FONTDIR/fonts.alias.new || exit 1
  mv $FONTDIR/fonts.alias.new $FONTDIR/fonts.alias || exit 1
  chmod 644 $FONTDIR/fonts.alias || exit 1
  chown bin.bin $FONTDIR/fonts.alias || exit 1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
