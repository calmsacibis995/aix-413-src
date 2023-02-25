# @(#)03	1.3  src/packages/devices/graphics/com/root/devices.graphics.com.unconfig.sh, pkgdevgraphics, pkg411, GOLD410 5/26/94 14:25:19
#   COMPONENT_NAME: PKGDEVGRAPHICS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# FUNCTION: script called from cleanup
#
# Removes the lft entry from /etc/inittab
#
rmitab "lft" >/dev/null 2>&1
return 0
