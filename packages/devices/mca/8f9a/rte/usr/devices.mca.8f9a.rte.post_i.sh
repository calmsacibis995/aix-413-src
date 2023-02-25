#  @(#)14        1.1  src/packages/devices/mca/8f9a/rte/usr/devices.mca.8f9a.rte.post_i.sh, pkgneptune, pkg411, 9436B411a 9/6/94 20:57:31
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: devices.mca.8f9a.rte.post_i.sh
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
#

if [ -x "devices.mca.8f9a.rte.ccm.odmdel" ] ; then
	devices.mca.8f9a.rte.ccm.odmdel
	if [ $? -ne 0 ] ; then
		inuumsg 146 $0 devices.mca.8f9a.rte.ccm.odmdel
		exit 1
	fi
fi

exit 0

