#! /bin/ksh
# @(#)27        1.2  src/packages/devices/pcmcia/a4000200/rte/usr/devices.pcmcia.a4000200.rte.pre_d.sh, pkgrspc, pkg411, 9437A411a 9/8/94 11:35:37
#
#   COMPONENT_NAME: pkgrspc
#
#   FUNCTIONS: none
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
# To adapt this script for a particular package option, edit the "uniqutypes"
# line below to define the uniquetypes which are defined in the PdDv for this
# package option.  Use spaces to delimit the uniquetypes within a single line.
#
# Example:
#   uniquetypes="adapter/mca/device1 adapter/mca/device2 adapter/mca/device3"
#
# Return:
#   0   - if deinstall of package option IS permitted
#   1   - if deinstall of package option IS NOT permitted
#
uniquetypes="adapter/pcmcia/a4000200"

for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0
