#!/bin/ksh
# @(#)18	1.2  X11.fnt.defaultFonts.post_u.sh, pkgx, pkg41J 1/16/95 17:04:26
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#   OBJECT CODE ONLY SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
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

exit 0      # exit with pass value
