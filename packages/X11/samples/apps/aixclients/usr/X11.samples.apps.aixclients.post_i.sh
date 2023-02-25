#!/bin/ksh
#  @(#)31        1.2  src/packages/X11/samples/apps/aixclients/usr/X11.samples.apps.aixclients.post_i.sh, pkgx, pkg411, GOLD410 6/7/94 11:02:53
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.apps.aixclients (usr) post installation configuration
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
clients="aixclock xscope"

for client in $clients; do
  rm -f $X11bin/$client
  ln -sf $samplesbin/$client $X11bin/$client
done

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
