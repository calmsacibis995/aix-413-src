#!/bin/ksh
#  @(#)84        1.3  src/packages/X11/samples/lib/Core/usr/X11.samples.lib.Core.unpost_i.sh, pkgx, pkg411, GOLD410 4/19/94 19:14:58
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.lib.Core (usr) unpost installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#   Remove customer courtesy links for sample libraries
#     Remove symbolic links to /usr/lib
#----------------------------------------------------------------
set -A lib_tgt	\
"/usr/lib/libXaw.a"	\
"/usr/lib/libXmu.a"

i=0
while ((i<${#lib_tgt[*]})) ; do
  if [ -L ${lib_tgt[i]} ]; then
    rm -f ${lib_tgt[i]}
  fi
  ((i=i+1))
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
