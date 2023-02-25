# @(#)03        1.1 8/12/94 src/packages/devices/sys/wga/rte/usr/devices.sys.wga.rte.post_i.sh, pkgdevgraphics, pkg411, 9433B411a 14:53:33
#
# COMPONENT_NAME: (bbldd) BBLDD Graphics Adapter
#
# FUNCTIONS: Remove the "frs" attribute
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

if [ -x "devices.sys.wga.rte.ccm.odmdel" ] ; then
	devices.sys.wga.rte.ccm.odmdel
	if [ $? -ne 0 ] ; then
		inuumsg 146 $0 devices.sys.wga.rte.ccm.odmdel
		exit 1
	fi
fi

exit 0

