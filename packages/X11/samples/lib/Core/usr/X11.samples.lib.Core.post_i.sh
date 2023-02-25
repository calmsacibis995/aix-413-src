#!/bin/ksh
#  @(#)83        1.1  src/packages/X11/samples/lib/Core/usr/X11.samples.lib.Core.post_i.sh, pkgx, pkg411, GOLD410 4/19/94 18:20:08
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.lib.Core (usr) post installation configuration
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
#   Add customer courtesy links for sample libraries
#     Add symbolic links to /usr/lib
#       * lib_src and lib_tgt arrays must match 1-to-1
#----------------------------------------------------------------
set -A lib_src	\
"/usr/lpp/X11/Xamples/lib/Xaw/libXaw.a"	\
"/usr/lpp/X11/Xamples/lib/Xmu/libXmu.a"

set -A lib_tgt	\
"/usr/lib/libXaw.a"	\
"/usr/lib/libXmu.a"

i=0
while ((i<${#lib_src[*]})) ; do
  if [ ! -f ${lib_tgt[i]} ]; then
    ln -sf ${lib_src[i]} ${lib_tgt[i]}
  fi
  ((i=i+1))
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
