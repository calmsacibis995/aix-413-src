#!/bin/ksh
#  @(#)98        1.5  src/packages/X11/base/rte/root/X11.base.rte.post_i.sh, pkgx, pkg411, GOLD410 4/22/94 14:21:05
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.base.rte (root) post installation configuration
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
#    Add config rule for /etc/methods/load_blockset_ext
#----------------------------------------------------------------

odmadd << END_X_CONFIG_RULES
Config_Rules:
	phase = 2
	seq = 0
	rule = "/etc/methods/load_blockset_ext"
END_X_CONFIG_RULES
if [ $? -ne 0 ]; then
  exit 1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
