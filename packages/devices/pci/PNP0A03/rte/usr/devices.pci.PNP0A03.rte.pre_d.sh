#! /bin/ksh
# @(#)34        1.1  src/packages/devices/pci/PNP0A03/rte/usr/devices.pci.PNP0A03.rte.pre_d.sh, pkgrspc, pkg41J, 9520A_all 5/12/95 10:05:23
#
#   COMPONENT_NAME: pkgrspc
#
#   FUNCTIONS: Determine whether Customized device entry exists
#			   for a given device package option
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
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
uniquetypes="bus/pci/pci"
for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0

