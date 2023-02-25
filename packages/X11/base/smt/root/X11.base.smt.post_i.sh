#!/bin/ksh
#  @(#)14        1.1  src/packages/X11/base/smt/root/X11.base.smt.post_i.sh, pkgx, pkg411, GOLD410 4/22/94 14:18:09
#
#   COMPONENT_NAME: pkgx
#
#   FUNCTIONS: X11.base.smt (root) post installation configuration
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
#    Add config rule for /etc/methods/startsmt
#----------------------------------------------------------------

odmadd << END_X_CONFIG_RULES
Config_Rules:
	phase = 2
	seq = 26
	rule = "/etc/methods/startsmt"
END_X_CONFIG_RULES
if [ $? -ne 0 ]; then
  exit 1
fi

#----------------------------------------------------------------
#  return to instal with successful return code
#----------------------------------------------------------------

exit 0      # exit with pass value
