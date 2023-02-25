# @(#)02        1.1 8/12/94 src/packages/devices/sys/wga/rte/usr/devices.sys.wga.rte.ccm.odmdel.sh, pkgdevgraphics, pkg411, 9433B411a 14:53:24
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

#
# Save off any ODM entries pertaining to this device that need to
# be put back on when rejected. But, do not run this if the ODM
# stuff has already been saved before which is the case when a
# Force (-F) install occurs
#

if [ ! -r "devices.sys.wga.rte.ccm.rodmadd" ]; then
	odmget  -q " uniquetype = 'adapter/sys/wga' " PdDv >> $SAVEDIR/devices.sys.wga.rte.ccm.rodmadd
	odmget  -q " uniquetype = 'adapter/sys/wga' " PdAt >> $SAVEDIR/devices.sys.wga.rte.ccm.rodmadd
	odmget  -q " uniquetype = 'adapter/sys/wga' " PdCn >> $SAVEDIR/devices.sys.wga.rte.ccm.rodmadd
	chmod a+x $SAVEDIR/devices.sys.wga.rte.ccm.rodmadd 
	odmdelete -o PdDv -q " uniquetype = 'adapter/sys/wga' " >/dev/null
	odmdelete -o PdAt -q " uniquetype = 'adapter/sys/wga' " >/dev/null
	odmdelete -o PdCn -q " uniquetype = 'adapter/sys/wga' " >/dev/null
fi
