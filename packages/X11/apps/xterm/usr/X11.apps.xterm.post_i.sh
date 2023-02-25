#!/bin/ksh
#  @(#)59        1.2  src/packages/X11/apps/xterm/usr/X11.apps.xterm.post_i.sh, pkgx, pkg411, GOLD410 5/12/94 16:58:22
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.apps.xterm (usr) post installation configuration
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
OPTION=X11.apps.xterm
grep $OPTION $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.41cfgfiles $OPTION.cfgfiles
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
