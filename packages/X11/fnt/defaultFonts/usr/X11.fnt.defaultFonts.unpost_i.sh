#!/bin/ksh
#  @(#)39        1.6  src/packages/X11/fnt/defaultFonts/usr/X11.fnt.defaultFonts.unpost_i.sh, pkgx, pkg411, GOLD410 5/11/94 08:57:59
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.defaultFonts (usr) unpost installation configuration
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
#   Restore default fonts.alias
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts
if [ -f $FONTDIR/fonts.alias ]; then
  grep -vxf $FONTDIR/fonts.alias.default $FONTDIR/fonts.alias > $FONTDIR/fonts.alias.new || exit 1
  mv $FONTDIR/fonts.alias.new $FONTDIR/fonts.alias || exit 1
  chmod 644 $FONTDIR/fonts.alias || exit 1
  chown bin.bin $FONTDIR/fonts.alias || exit 1
fi

#----------------------------------------------------------------
#   Restore Type1 fonts.scale
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts/Type1
if [ -f $FONTDIR/fonts.scale ]; then
  grep -vxf $FONTDIR/fonts.scale.default $FONTDIR/fonts.scale > $FONTDIR/fonts.scale.new || exit 1

  # Check count in resulting fonts.scale.new.
  ((scale_count=`wc -l $FONTDIR/fonts.scale.new | sed "s/^ *//" | cut -f1 -d" "` - 1))
  echo $scale_count > $FONTDIR/fonts.scale || exit 1
  awk '{
         if ( NR != 1 ) {
           print $0
         }
       }' $FONTDIR/fonts.scale.new >> $FONTDIR/fonts.scale || exit 1
  chmod 644 $FONTDIR/fonts.scale || exit 1
  chown bin.bin $FONTDIR/fonts.scale || exit 1
  rm -f $FONTDIR/fonts.scale.new || exit 1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
