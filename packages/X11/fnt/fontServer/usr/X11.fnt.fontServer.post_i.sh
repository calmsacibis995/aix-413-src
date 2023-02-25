#!/bin/ksh
#  @(#)98        1.1  src/packages/X11/fnt/fontServer/usr/X11.fnt.fontServer.post_i.sh, pkgx, pkg411, GOLD410 5/13/94 09:01:38
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.fnt.fontServer (usr) post installation configuration
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
#  Determine migration level cfgfiles required
#----------------------------------------------------------------
OPTION=X11.fnt.fontServer
grep $OPTION $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.41cfgfiles $OPTION.cfgfiles
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
