#!/bin/bsh
# @(#)58        1.1  src/packages/bos/loc/utf/ZH_CN/usr/bos.loc.utf.ZH_CN.post_i.sh, pkglociso, pkg41J, 9512A_all 3/16/95 21:15:10
#
#   COMPONENT_NAME: pkglociso
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#   Create fonts.dir files for the bos.loc.iso.ZH_CN fonts
#----------------------------------------------------------------

fontdirlist="/usr/lpp/X11/lib/X11/fonts"

# We depend on the mkfontdir executable to already have been restored,
# and to already have execute permission by virtue of the sysck run on
# it at installation image creation time.

for dir in $fontdirlist
do

   if [ -f "$dir/fonts.dir" ]
   then
      rm -f $dir/fonts.dir || exit 1
   fi

   if [ -x /usr/lpp/X11/bin/mkfontdir ]
   then
      /usr/lpp/X11/bin/mkfontdir $dir > /dev/null 2>&1 || exit 1
   fi

done

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
#  return to install with successful return code
#----------------------------------------------------------------

exit 0 		# exit with pass value
