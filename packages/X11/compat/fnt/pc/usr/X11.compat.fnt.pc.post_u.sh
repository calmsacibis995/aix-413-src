#!/bin/ksh
# @(#)34	1.2  src/packages/X11/compat/fnt/pc/usr/X11.compat.fnt.pc.post_u.sh, pkgx, pkg41B, 412_41B_sync 1/16/95 17:05:18
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
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
