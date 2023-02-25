#!/bin/ksh
# @(#)08	1.2  src/packages/X11/fnt/ucs/cjk/usr/X11.fnt.ucs.cjk.post_i.sh, ils-zh_CN, pkg41J, 9513A_all 3/24/95 16:47:57
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#   Create fonts.dir file for the X11 fonts
#----------------------------------------------------------------
FONTDIR=/usr/lpp/X11/lib/X11/fonts
if [ -f $FONTDIR/fonts.dir ]; then
  rm -f $FONTDIR/fonts.dir || exit 1
fi
if [ -x /usr/lpp/X11/bin/mkfontdir ]; then
  /usr/lpp/X11/bin/mkfontdir $FONTDIR >/dev/null 2>&1 || exit 1
fi

#---------------------------------------------------------------+
# Now set up the fonts.alias file                               |
#---------------------------------------------------------------+

FONTDIR=/usr/lpp/X11/lib/X11/fonts

if [ ! -f "$FONTDIR/fonts.alias" ]
then
   echo "FILE_NAMES_ALIASES" > $FONTDIR/fonts.alias || exit 1
fi

cat $FONTDIR/fonts.alias.ZH >> $FONTDIR/fonts.alias || exit 1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value

