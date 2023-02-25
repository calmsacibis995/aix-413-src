#!/bin/bsh
# @(#)23        1.3  src/packages/bos/txt/hplj/fnt/usr/bos.txt.hplj.fnt.unpost_i.sh, pkgtxt, pkg411, GOLD410 6/3/94 12:40:42
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
# NAME: bos.txt.hplj.fnt.unpost_i
#
# FUNCTION: script called from instal to do special configuration cleanup 
#           for hplj downloaded font support
#
#
# RETURN VALUE DESCRIPTION:
#		The return code from the makedev command
#          
##
#
##########
## Restore the original printer description file
##########

LC_MESSAGES=$LANG
LANG=C
export LANG LC_MESSAGES

cd /usr/lib/font/devhplj

##
##  Save a copy of the DESC file.
##

mv DESC.res DESC

/usr/bin/makedev DESC 2>/dev/null

#remove bitmap directory
rm -rf bitmaps

exit $?
