#! /bin/ksh
# @(#)39        1.1 4/27/94 src/packages/devices/mca/efc6/rte/root/devices.mca.efc6.rte.post_i.sh, pkgdevgraphics, pkg411, GOLD410 09:51:52
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: devices.mca.efc6.rte (root) post installation configuration
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

/usr/lpp/devices.mca.efc6/__bbl_efc6_post_i ddins bbldd_mca

if [ $? -ne 0 ]; then
  exit 1
fi

rm -f /usr/lpp/devices.mca.efc6/__bbl_efc6_post_i > /dev/null 2> /dev/null

exit 0
