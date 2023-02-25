#! /bin/ksh
# @(#)08        1.6  src/packages/devices/scsi/disk/rspc/usr/devices.scsi.disk.rspc.pre_d.sh, pkgrspc, pkg41J, 9517B_all 4/27/95 10:55:53
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
uniquetypes="disk/scsi/270mb disk/scsi/270mb2 disk/scsi/340mb disk/scsi/360mb disk/scsi/540mb2 disk/scsi/540mb3 disk/scsi/540mb4 disk/scsi/540mb5 disk/scsi/730mb2 disk/scsi/810mb disk/scsi/1000mb2 disk/scsi/1080mb cdrom/scsi/enhcdrom2 cdrom/scsi/enhcdrom4"
for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0

