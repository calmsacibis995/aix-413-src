#!/bin/bsh
# @(#)08        1.1  src/packages/bos/txt/bib/usr/bos.txt.bib.post_i.sh, pkgtxt, pkg411, GOLD410 12/23/92 09:07:24
#
# COMPONENT_NAME: pkgtxt
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993.
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
##
# NAME: bos.txt.bib.post_i
#
# FUNCTION: script called from instal to do special configuration for
#           Bibliography Support Tools
#
#
# RETURN VALUE DESCRIPTION:
#          return code from indxbib
##
#
##########
## Build Ind.ia Ind.ib Ind.ic files for refer
##########

cd /usr/share/dict/papers

/usr/bin/indxbib Ind

exit $?
