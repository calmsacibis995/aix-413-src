#!/bin/ksh
# @(#)23	1.2  src/packages/X11/fnt/iso1/usr/X11.fnt.iso1.post_u.sh, pkgx, pkg41B, 412_41B_sync 1/16/95 17:04:41
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
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
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
