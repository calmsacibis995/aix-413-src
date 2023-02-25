#! /bin/ksh
# @(#)27	1.8  src/packages/devices/scsi/disk/rte/usr/devices.scsi.disk.rte.pre_d.sh, pkgdevbase, pkg411, 9432A411a 8/5/94 08:34:55
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
uniquetypes="disk/scsi/1000mb disk/scsi/1000mbde disk/scsi/1200mb disk/scsi/1370mb disk/scsi/160mb disk/scsi/2000mb disk/scsi/2000mbde disk/scsi/200mb disk/scsi/320mb disk/scsi/355mb disk/scsi/400mb disk/scsi/540mb disk/scsi/670mb disk/scsi/730mb disk/scsi/857mb disk/scsi/osdisk disk/scsi/2000mb16bitde disk/scsi/2000mb16bit rwoptical/scsi/osomd cdrom/scsi/cdrom1 cdrom/scsi/oscd cdrom/scsi/enhcdrom disk/scsi/1000mb16bitde"

for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done

uniquetypes="disk/scsi/1000mb16bit disk/scsi/1100mb disk/scsi/1100mb16bit disk/scsi/1100mb16bitde disk/scsi/2200mb disk/scsi/2200mb16bit disk/scsi/2200mb16bitde disk/scsi/4500mb16bit disk/scsi/4500mb16bitde cdrom/scsi/enhcdrom3 cdrom/scsi/scsd disk/scsi/scsd rwoptical/scsi/scsd"

for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0

