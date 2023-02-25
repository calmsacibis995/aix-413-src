#!/bin/ksh
#  @(#)36        1.5  src/packages/X11/loc/sv_SE/base/lib/usr/X11.loc.sv_SE.base.lib.post_i.sh, pkgx, pkg411, 9437A411a 9/12/94 15:18:57
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.loc.sv_SE.base.lib (usr) post installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------
#  Determine migration level cfgfiles required
#----------------------------------------------------------------
OPTION=X11.loc.sv_SE.base.lib
grep $OPTION $OPTION.installed_list > /dev/null 2>&1
if [ $? -eq 0 ]; then
  mv $OPTION.41cfgfiles $OPTION.cfgfiles
fi

#----------------------------------------------------------------
#  Integrate Customiziing Tool into Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/custom -l sv_SE > /dev/null 2>&1

#----------------------------------------------------------------
#  Integrate X examples into Dt
#----------------------------------------------------------------
/etc/dtappintegrate -s /usr/lpp/X11/Xamples -l sv_SE > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
