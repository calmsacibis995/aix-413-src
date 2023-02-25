#!/bin/bsh
# @(#)21        1.3  src/packages/bos/txt/hplj/fnt/usr/bos.txt.hplj.fnt.post_i.sh, pkgtxt, pkg411, GOLD410 6/3/94 12:40:39
#
#   COMPONENT_NAME: pkgtxt
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
##
# NAME: bos.txt.hplj.fnt.post_i
#
# FUNCTION: script called from instal to do special configuration for
#           hplj downloaded font support
#
#
# RETURN VALUE DESCRIPTION:
#		The return code from the makedev command
#          
##
#
##########
## Modify the printer description file to support the full set of 
## canonical troff font sizes and to use the downloaded special font
## instead of the printer resident specials fonts.
##########

LC_MESSAGES=$LANG
LANG=C
export LANG LC_MESSAGES

cd /usr/lib/font/devhplj

##
##  Save a copy of the DESC file.
##

mv DESC DESC.res

##
##	Edit the DESC file
##

cat DESC.res | sed -e \
"/fonts/c\\
fonts 8 R I B C CB sp SPC SM7" \
-e "/sizes/c\\
sizes 6 7 8 9 10 11 12 14 16 18 20 22 24 28 36 0"  > DESC 2>/dev/null

/usr/bin/makedev DESC 2>/dev/null

exit $?
