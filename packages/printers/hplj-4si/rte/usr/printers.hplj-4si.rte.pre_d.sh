#!/usr/bin/ksh
# @(#)22        1.1  src/packages/printers/hplj-4si/rte/usr/printers.hplj-4si.rte.pre_d.sh, pkgdevprint, pkg41J, 9512A_all 3/15/95 19:13:37
#
#   COMPONENT_NAME: pkgdevprint
#
#   FUNCTIONS: Determine whether Customized device entry exists
#			   for a given printer package option
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
# Return:  
#    		0    if deinstall of package option IS permitted
#    		1    if deinstall of package option IS NOT permitted
#
typeset -r	uniquetypes="printer/parallel/hplj-4si printer/rs232/hplj-4si printer/rs422/hplj-4si"
typeset -r	ptype="hplj-4si"
typeset		type

# Check for any configured printer devices.
for type in $uniquetypes
do
	[[ -n "$(odmget -q PdDvLn=$type CuDv)" ]] && exit 1
done

# Check for any configured virtual printers.
ls /var/spool/lpd/pio/@local/custom/*:* 2>/dev/null|xargs fgrep :mt:| \
	egrep ":${ptype}[ 	]*$" >/dev/null && exit 1

exit 0
