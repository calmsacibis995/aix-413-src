#!/bin/bsh
# @(#)68	1.4  src/packages/bos/loc/com/JP/usr/bos.loc.com.JP.post_i.sh, pkgloccom, pkg411, GOLD410 3/24/94 10:41:18
#
# COMPONENT_NAME: pkgloccom
#
# FUNCTIONS: post_install
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: bos.loc.com.JP.post_i.sh
#
# FUNCTION: script called from instal script to create the fonts.dir
#           and fonts.alias files for the bos.loc.com.JP fonts.
#
# RETURN VALUE DESCRIPTION:
#        0 if all options succeed
#

#----------------------------------------------------------------
#   Create fonts.dir files for the bos.loc.com.JP fonts
#----------------------------------------------------------------

fontdirlist="\
	/usr/lpp/X11/lib/X11/fonts \
	/usr/lpp/X11/lib/X11/fonts/JP \
	/usr/lpp/X11/lib/X11/fonts/misc"

# We depend on the mkfontdir executable to already have been restored,
# and to already have execute permission by virtue of the sysck run on
# it at installation image creation time.

for dir in $fontdirlist; do

  if [ -f "$dir/fonts.dir" ]
  then rm -f $dir/fonts.dir || exit 1
  fi

  if [ -x /usr/lpp/X11/bin/mkfontdir ]
  then /usr/lpp/X11/bin/mkfontdir $dir > /dev/null 2>&1 || exit 1
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

cat $FONTDIR/fonts.alias.JP >> $FONTDIR/fonts.alias || exit 1

#----------------------------------------------------------------
#  return to install with successful return code
#----------------------------------------------------------------

exit 0 		# exit with pass value
