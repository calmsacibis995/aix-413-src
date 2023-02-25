#!/bin/ksh
# @(#)29        1.1  src/packages/bos/txt/tfs/usr/bos.txt.tfs.unpost_i.sh, pkgtxt, pkg411, GOLD410 12/23/92 09:08:12
#
# COMPONENT_NAME: pkgtxt
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
##
# NAME: bos.txt.tfs.unpost_i
#
# FUNCTION: script called from to do cleanup for Text Formatting
#
#
# RETURN VALUE DESCRIPTION:
#		always 0
#          
###########
FONTDIR=/usr/lib/font
typeset -i rc=0

trap "" 1 2 3 15

# Restore original DESC file
if [ -r $FONTDIR/devhplj/DESC.res ] ; then
	cd $FONTDIR/devhplj
	mv DESC.res DESC 2>/dev/null
fi

if [ -r $FONTDIR/devibm3812/DESC.res ] ; then
	cd $FONTDIR/devibm3812
	mv DESC.res DESC 2>/dev/null
fi

if [ -r $FONTDIR/devibm3816/DESC.res ] ; then
	cd $FONTDIR/devibm3816
	mv DESC.res DESC 2>/dev/null
fi

exit $rc
