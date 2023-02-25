#!/bin/ksh
#  @(#)04        1.1  src/packages/X11/loc/no_NO/base/rte/usr/X11.loc.no_NO.base.rte.pre_i.sh, pkgx, pkg411, 9439C411d 9/29/94 14:40:07
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.loc.no_NO.base.rte (usr) pre installation configuration
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
#  Remove extraneous rgb.txt file
#----------------------------------------------------------------
ODMDIR=/usr/lib/objrepos
rm -f /usr/lpp/X11/lib/X11/no_NO/rgb.txt
odmdelete -q"loc0='/usr/lpp/X11/lib/X11/no_NO/rgb.txt'" inventory > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
