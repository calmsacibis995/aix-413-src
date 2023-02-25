#!/bin/bsh
# @(#)55        1.1  src/packages/bos/loc/iso/zh_CN/usr/bos.loc.iso.zh_CN.unpost_i.sh, pkglociso, pkg41J, 9512A_all 3/15/95 21:37:30
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
#---------------------------------------------------------------+
#  Now restore the fonts.alias file                             |
#---------------------------------------------------------------+

FONTDIR=/usr/lpp/X11/lib/X11/fonts

if [ -f "$FONTDIR/fonts.alias" ]
then
   grep -vxf $FONTDIR/fonts.alias.CN $FONTDIR/fonts.alias > $FONTDIR/fonts.new$$ || [ $$? -lt 2 ] || exit 1
   rm -f $FONTDIR/fonts.alias || exit 1
   mv $FONTDIR/fonts.new$$ $FONTDIR/fonts.alias || exit 1
   chown bin.bin $FONTDIR/fonts.alias || exit 1
   chmod 644 $FONTDIR/fonts.alias || exit 1

   #---------------------------------------------------------------+
   # If there is just the one header line in the fonts.alias at    |
   # this point, then remove the fonts.alias file altogether       |
   #---------------------------------------------------------------+

   echo "FILE_NAMES_ALIASES" > $FONTDIR/empty.fonts$$ || exit 1
   cmp -s $FONTDIR/empty.fonts$$ $FONTDIR/fonts.alias
   if [ $? -eq 0 ]
   then
      rm -f $FONTDIR/fonts.alias || exit 1
   fi

   rm -f $FONTDIR/empty.fonts$$ || exit 1
 
fi

#----------------------------------------------------------------
#  return to install with successful return code
#----------------------------------------------------------------

exit 0 		# exit with pass value
