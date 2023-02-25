#! /bin/ksh
# @(#)26	1.2  src/packages/devices/mca/8fd4/rte/usr/devices.mca.8fd4.rte.pre_d.sh, pkgdevbase, pkg411, GOLD410 1/25/94 17:26:51
#
#   COMPONENT_NAME: pkgdevbase
#
#   FUNCTIONS: Determine whether Customized device entry exists
#			   for a given device package option
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
# To adapt this script for a particular package option edit the "uniqutypes"
# line below to define the uniquetypes which are defined in the PdDv for
# this package option.
# Use spaces to delimit the uniquetypes within a single line
# Example:
# uniquetypes="adapter/mca/device1 adapter/mca/device2 adapter/mca/device3"
#
# Return:  
#    		0    if deinstall of package option IS permitted
#    		1    if deinstall of package option IS NOT permitted
#
uniquetypes="adapter/mca/8fd4"
for type in $uniquetypes
do
	rc=$(odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0

