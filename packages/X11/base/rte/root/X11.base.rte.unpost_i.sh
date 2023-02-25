#  @(#)99        1.4  src/packages/X11/base/rte/root/X11.base.rte.unpost_i.sh, pkgx, pkg411, GOLD410 4/22/94 14:21:53
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.base.rte (root) post installation unconfiguration
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
#    Remove config rule for /etc/methods/load_blockset_ext
#----------------------------------------------------------------

odmdelete -q "rule='/etc/methods/load_blockset_ext'" -o Config_Rules > /dev/null 2>&1

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
