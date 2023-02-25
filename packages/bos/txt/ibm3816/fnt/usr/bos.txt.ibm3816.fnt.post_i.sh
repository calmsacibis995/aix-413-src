#!/bin/bsh
# @(#)99        1.3  src/packages/bos/txt/ibm3816/fnt/usr/bos.txt.ibm3816.fnt.post_i.sh, pkgtxt, pkg411, GOLD410 6/3/94 12:40:51
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
#
##
# NAME: bos.txt.ibm3816.fnt.post_i
#
# FUNCTION: script called from instal to do special configuration for
#           ibm3816 downloaded font support
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

cd /usr/lib/font/devibm3816

##
##  Save a copy of the DESC file.
##

mv DESC DESC.res

##
##	Edit the DESC file
##

cat DESC.res | sed -e \
"/fonts/c\\
fonts 6 R I B sp S S2" \
-e "/sizes/c\\
sizes 6 7 8 9 10 11 12 14 16 18 20 22 24 28 36 0"  > DESC 2>/dev/null

/usr/bin/makedev DESC 2>/dev/null

exit $?
