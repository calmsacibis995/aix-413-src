#! /bin/ksh
# @(#)79        1.1  src/packages/bos/dlc/8023/usr/bos.dlc.8023.pre_d.sh, pkgdlc, pkg411, GOLD410 3/24/94 18:25:34
#
#   COMPONENT_NAME: pkgdlc
#
#   FUNCTIONS: Determine whether Customized device entry exists
#                          for a given device package option
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
#               0    if deinstall of package option IS permitted
#               1    if deinstall of package option IS NOT permitted
#
uniquetypes="dlc/dlc/IEEE_ethernet"
for type in $uniquetypes
do
	rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
	if [ -n "$rc" ]
	then
		exit 1
	fi
done
exit 0
