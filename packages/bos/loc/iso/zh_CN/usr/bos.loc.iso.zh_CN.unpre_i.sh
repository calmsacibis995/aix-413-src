#!/bin/bsh
# @(#)52	1.1  src/packages/bos/loc/iso/zh_CN/usr/bos.loc.iso.zh_CN.unpre_i.sh, ils-zh_CN, pkg41B, 9504A 12/22/94 13:12:22
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
#   Create fonts.dir files for the bos.loc.iso.zh_CN fonts
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
