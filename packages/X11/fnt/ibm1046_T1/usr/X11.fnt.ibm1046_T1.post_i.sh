#!/bin/ksh
#  @(#)93        1.4  src/packages/X11/fnt/ibm1046_T1/usr/X11.fnt.ibm1046_T1.post_i.sh, pkgx, pkg411, GOLD410 3/16/94 11:10:18
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.ibm1046_T1 (usr) post installation configuration
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
  /usr/lpp/X11/bin/mkfontdir $FONTDIR > /dev/null 2>&1 || exit 1
fi

#----------------------------------------------------------------
#   Add iso entries to X11 Type1 fonts.scale
#----------------------------------------------------------------
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
     }' $FONTDIR/fonts.scale $FONTDIR/fonts.scale.1046 > $FONTDIR/fonts.new
cat $FONTDIR/fonts.count $FONTDIR/fonts.new > $FONTDIR/fonts.scale || exit 1
chmod 644 $FONTDIR/fonts.scale || exit 1
chown bin.bin $FONTDIR/fonts.scale || exit 1
rm -f $FONTDIR/fonts.count $FONTDIR/fonts.new || exit 1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
