#!/bin/bsh
# @(#)88	1.2  src/packages/bos/loc/iso/zh_TW/usr/bos.loc.iso.zh_TW.unpre_i.sh, pkglociso, pkg411, GOLD410 3/6/94 23:25:22
#
#   COMPONENT_NAME: pkglociso
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#   Create fonts.dir files for the bos.loc.iso.zh_TW fonts
#----------------------------------------------------------------

fontdirlist="/usr/lpp/X11/lib/X11/fonts"

# We depend on the mkfontdir executable to already have been restored,
# and to already have execute permission by virtue of the sysck run on
# it at installation image creation time.

for dir in $fontdirlist; do

  if [ -f "$dir/fonts.dir" ]
  then
     rm -f $dir/fonts.dir || exit 1
  fi

  if [ -x /usr/lpp/X11/bin/mkfontdir ]
  then
     /usr/lpp/X11/bin/mkfontdir $dir > /dev/null 2>&1 || exit 1
  fi

done

#----------------------------------------------------------------
#  return to install with successful return code
#----------------------------------------------------------------

exit 0 		# exit with pass value
