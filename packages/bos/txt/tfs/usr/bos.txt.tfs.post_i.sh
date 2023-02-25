#!/bin/ksh
# @(#)28        1.2  src/packages/bos/txt/tfs/usr/bos.txt.tfs.post_i.sh, pkgtxt, pkg411, GOLD410 4/2/93 14:49:51
#
# COMPONENT_NAME: pkgtxt
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
##
# NAME: bos.txt.tfs.post_i
#
# FUNCTION: script called from installp to do post-install configuration for
#           Text Formatting.
#
#
# RETURN VALUE DESCRIPTION:
#		The return code from the makedev command
#          
##
## If downloadable fonts for a printer are already installed AND
## not being reinstalled along with txtfmt.tfs.obj then
## reconfigure the downloadable font description file (DESC).
###########
FONTDIR=/usr/lib/font
typeset -i rc=0
OLDCWD=$(pwd)

trap "" 1 2 3 15
##########
## Modify the printer description file to support the full set of 
## canonical troff font sizes and to use the downloaded special font
## instead of the printer resident specials fonts.
##########


if [[ -d $FONTDIR/devhplj/bitmaps ]] && [[ -r $INUTEMPDIR/master.al ]] &&
   [[ $(grep -c "$FONTDIR/devhplj/bitmaps" $INUTEMPDIR/master.al) -eq 0 ]]
then

	cd /usr/lib/font/devhplj
	mv DESC DESC.res
	cat DESC.res | sed -e \
	"/fonts/c\\
	fonts 8 R I B C CB sp SPC SM7" \
	-e "/sizes/c\\
	sizes 6 7 8 9 10 11 12 14 16 18 20 22 24 28 36 0"  > DESC 2>/dev/null
	/usr/bin/makedev DESC 2>/dev/null
	rc=$?
	cd $OLDCWD
	if [[ $rc -ne 0 ]] ; then
		exit $rc
	fi
fi

if [[ -d $FONTDIR/devibm3812/bitmaps ]] && [[ -r $INUTEMPDIR/master.al ]] &&
   [[ $(grep -c "$FONTDIR/devibm3812/bitmaps" $INUTEMPDIR/master.al) -eq 0 ]]
then

	cd $FONTDIR/devibm3812
	mv DESC DESC.res
	cat DESC.res | sed -e \
	"/fonts/c\\
	fonts 9 R I B C sp S S1 S2 S3" \
	-e "/sizes/c\\
	sizes 6 7 8 9 10 11 12 14 16 18 20 22 24 28 36 0"  > DESC 2>/dev/null
	/usr/bin/makedev DESC 2>/dev/null
	rc=$?
	cd $OLDCWD
	if [[ $rc -ne 0 ]] ; then
		exit $rc
	fi
fi

if [[ -d $FONTDIR/devibm3816/bitmaps ]] && [[ -r $INUTEMPDIR/master.al ]] &&
   [[ $(grep -c "$FONTDIR/devibm3816/bitmaps" $INUTEMPDIR/master.al) -eq 0 ]]
then

	cd $FONTDIR/devibm3816
	mv DESC DESC.res
	cat DESC.res | sed -e \
	"/fonts/c\\
	fonts 6 R I B sp S S2" \
	-e "/sizes/c\\
	sizes 6 7 8 9 10 11 12 14 16 18 20 22 24 28 36 0"  > DESC 2>/dev/null
	/usr/bin/makedev DESC 2>/dev/null
	rc=$?
	cd $OLDCWD
	if [[ $rc -ne 0 ]] ; then
		exit $rc
	fi
fi

exit $rc
