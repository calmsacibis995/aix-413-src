#!/bin/ksh
#  @(#)41        1.9  src/packages/X11/fnt/defaultFonts/usr/X11.fnt.defaultFonts.post_i.sh, pkgx, pkg411, GOLD410 5/10/94 08:33:33
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.defaultFonts (usr) post installation configuration
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
fontdirlist="/usr/lpp/X11/lib/X11/fonts \
			/usr/lpp/X11/lib/X11/fonts/100dpi \
			/usr/lpp/X11/lib/X11/fonts/ibm850 \
			/usr/lpp/X11/lib/X11/fonts/Type1"

for dir in $fontdirlist; do
  if [ -f $dir/fonts.dir ]; then
    rm -f $dir/fonts.dir || exit 1
  fi
  if [ -x /usr/lpp/X11/bin/mkfontdir ]; then
    /usr/lpp/X11/bin/mkfontdir $dir > /dev/null 2>&1 || exit 1
  fi
done

#----------------------------------------------------------------
#   Set up default fonts.alias
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts
if [ ! -f $FONTDIR/fonts.alias ]; then
  echo "FILE_NAMES_ALIASES" > $FONTDIR/fonts.alias || exit 1
fi
cat $FONTDIR/fonts.alias.default >> $FONTDIR/fonts.alias || exit 1

#----------------------------------------------------------------
#   Set up 100dpi and ibm850 fonts.alias
#----------------------------------------------------------------
fontaliaslist=" /usr/lpp/X11/lib/X11/fonts/100dpi \
			/usr/lpp/X11/lib/X11/fonts/ibm850"

for dir in $fontaliaslist; do
  if [ ! -f $dir/fonts.alias ]; then
    echo "FILE_NAMES_ALIASES" > $dir/fonts.alias || exit 1
  fi
done

#----------------------------------------------------------------
#   Set up Type1 fonts.scale
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts/Type1
if [ ! -f $FONTDIR/fonts.scale ]; then
  echo "0" > $FONTDIR/fonts.scale || exit 1
fi
awk '{
       if ( NR != 1 ) {
         print $0
       }
     }
     END {
       system( "echo " NR-1 " > '"$FONTDIR"'/fonts.count" )
     }' $FONTDIR/fonts.scale $FONTDIR/fonts.scale.default > $FONTDIR/fonts.new
cat $FONTDIR/fonts.count $FONTDIR/fonts.new > $FONTDIR/fonts.scale || exit 1
chmod 644 $FONTDIR/fonts.scale || exit 1
chown bin.bin $FONTDIR/fonts.scale || exit 1
rm -f $FONTDIR/fonts.count $FONTDIR/fonts.new || exit 1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
