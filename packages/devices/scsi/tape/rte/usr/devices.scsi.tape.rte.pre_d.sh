#! /bin/ksh
# @(#)28	1.4  src/packages/devices/scsi/tape/rte/usr/devices.scsi.tape.rte.pre_d.sh, pkgdevbase, pkg411, 9434A411a 8/22/94 11:05:03
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
uniquetypes="tape/scsi/1200mb-c tape/scsi/150mb tape/scsi/4mm2gb tape/scsi/525mb tape/scsi/8mm tape/scsi/8mm5gb tape/scsi/8mm7gb tape/scsi/9trk tape/scsi/ost tape/scsi/4mm4gb tape/scsi/3490e"
for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0

