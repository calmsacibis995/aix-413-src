#  @(#)13        1.1  src/packages/devices/mca/8f9a/rte/usr/devices.mca.8f9a.rte.ccm.odmdel.sh, pkgneptune, pkg411, 9436B411a 9/6/94 20:54:09
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: Remove the "frs" attribute
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
# Save off any ODM entries pertaining to this device that need to
# be put back on when rejected. But, do not run this if the ODM
# stuff has already been saved before which is the case when a
# Force (-F) install occurs
#

if [ ! -r "devices.mca.8f9a.rte.ccm.rodmadd" ]; then
	odmget  -q " uniquetype = 'adapter/mca/nep' " PdDv >> $SAVEDIR/devices.mca.8f9a.rte.ccm.rodmadd
	odmget  -q " uniquetype = 'adapter/mca/nep' " PdAt >> $SAVEDIR/devices.mca.8f9a.rte.ccm.rodmadd
	odmget  -q " uniquetype = 'adapter/mca/nep' " PdCn >> $SAVEDIR/devices.mca.8f9a.rte.ccm.rodmadd
	chmod a+x $SAVEDIR/devices.mca.8f9a.rte.ccm.rodmadd
	odmdelete -o PdDv -q " uniquetype = 'adapter/mca/nep' " >/dev/null
	odmdelete -o PdAt -q " uniquetype = 'adapter/mca/nep' " >/dev/null
	odmdelete -o PdCn -q " uniquetype = 'adapter/mca/nep' " >/dev/null
fi

