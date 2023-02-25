#!/bin/ksh
#  @(#)39        1.3  src/packages/X11/apps/custom/usr/X11.apps.custom.post_i.sh, pkgx, pkg411, GOLD410 6/27/94 12:51:12
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.apps.custom (usr) post installation configuration
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
OPTION=X11.apps.custom
grep $OPTION $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.41cfgfiles $OPTION.cfgfiles
fi

#----------------------------------------------------------------
#  Integrate Customizing Tool into Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/custom -l C

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
