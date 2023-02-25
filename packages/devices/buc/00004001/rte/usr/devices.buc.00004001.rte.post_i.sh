# @(#)05        1.1 8/12/94 src/packages/devices/buc/00004001/rte/usr/devices.buc.00004001.rte.post_i.sh, pkgdevgraphics, pkg411, 9433B411a 14:57:19
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

if [ -x "devices.buc.00004001.rte.ccm.odmdel" ] ; then
	devices.buc.00004001.rte.ccm.odmdel
	if [ $? -ne 0 ] ; then
		inuumsg 146 $0 devices.buc.00004001.rte.ccm.odmdel
		exit 1
	fi
fi

exit 0

