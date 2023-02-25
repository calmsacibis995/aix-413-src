#! /bin/ksh
# @(#)50	1.2  src/packages/devices/rs6ksmp/base/rte/usr/devices.rs6ksmp.base.rte.pre_d.sh, pkgdevbase, pkg41J, 9511A_all 3/14/95 16:31:53
#
#   COMPONENT_NAME: pkgdevbase
#
#   FUNCTIONS: Determine whether Customized device entry exists
#			   for a given device package option
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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
uniquetypes="sys/node/sys_p planar/sys/sysplanar_p ioplanar/sys/iodplanar1 ioplanar/planar/mcaplanar1 container/sys/cabinet1 container/cabinet/sif1 container/cabinet/power_supply1 container/cabinet/op_panel1 container/sys/smpdrawer1"
for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0

