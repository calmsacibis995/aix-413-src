#!/bin/ksh
#  @(#)57        1.4  src/packages/X11/apps/xdm/usr/X11.apps.xdm.post_i.sh, pkgx, pkg411, GOLD410 6/20/94 16:44:23
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.apps.xdm (usr) post installation configuration
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
OPTION=X11.apps.xdm
grep "X11dev.src 1.2.0.0" $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.R4cfgfiles $OPTION.cfgfiles
else
  grep $OPTION $OPTION.installed_list > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mv $OPTION.41cfgfiles $OPTION.cfgfiles
  fi
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
