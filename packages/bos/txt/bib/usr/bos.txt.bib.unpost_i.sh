#!/bin/bsh
# @(#)10        1.1  src/packages/bos/txt/bib/usr/bos.txt.bib.unpost_i.sh, pkgtxt, pkg411, GOLD410 12/23/92 09:07:29
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
# NAME: bos.txt.bib.unpost_i
#
# FUNCTION: script called from instal to do configuration cleanup for
#           Bibliography Support Tools
#
#
#
##########
## Remove Ind.ia Ind.ib Ind.ic files for refer
##########

cd /usr/share/dict/papers

rm -f Ind.i[abc]

exit 0
