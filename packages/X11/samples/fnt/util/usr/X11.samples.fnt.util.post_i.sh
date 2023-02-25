#!/bin/ksh
#  @(#)27        1.3  src/packages/X11/samples/fnt/util/usr/X11.samples.fnt.util.post_i.sh, pkgx, pkg411, GOLD410 6/7/94 10:59:24
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.fnt.util (usr) post installation configuration
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
#   Add customer courtesy links for sample clients
#     Add symbolic links to /usr/lpp/X11/bin
#----------------------------------------------------------------
X11bin="/usr/lpp/X11/bin"
samplesbin="/usr/lpp/X11/Xamples/bin"
clients="fsinfo fslsfonts fstobdf showfont"

for client in $clients; do
  rm -f $X11bin/$client
  ln -sf $samplesbin/$client $X11bin/$client
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
