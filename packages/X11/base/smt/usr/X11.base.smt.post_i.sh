#!/bin/ksh
#  @(#)21        1.1  src/packages/X11/base/smt/usr/X11.base.smt.post_i.sh, pkgx, pkg411, GOLD410 4/22/94 14:18:42
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.base.smt (usr) post installation configuration
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
#  Load Shared Memory extension.  This will save the user from
#  having to reboot after installation.
#----------------------------------------------------------------
/usr/lib/drivers/loadsmt /usr/lib/drivers/smt_loadpin
/usr/lib/drivers/loadsmt /usr/lib/drivers/smt_load

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
