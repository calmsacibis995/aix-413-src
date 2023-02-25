#!/bin/ksh
#  @(#)02        1.2  src/packages/X11/samples/apps/demos/usr/X11.samples.apps.demos.post_i.sh, pkgx, pkg411, GOLD410 6/20/94 16:46:01
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.samples.apps.demos (usr) post installation configuration
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
OPTION=X11.samples.apps.demos
grep "X11dev.src 1.2.0.0" $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.R4cfgfiles $OPTION.cfgfiles
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
