#!/usr/bin/ksh
# @(#)41	1.1  src/packages/printers/canlbp-A404PS/rte/usr/printers.canlbp-A404PS.rte.pre_d.sh, pkgdevprint, pkg411, GOLD410 2/23/94 18:33:50
#
#   COMPONENT_NAME: pkgdevprint
#
#   FUNCTIONS: Determine whether Customized device entry exists
#			   for a given printer package option
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
# Return:  
#    		0    if deinstall of package option IS permitted
#    		1    if deinstall of package option IS NOT permitted
#
typeset -r	uniquetypes="printer/parallel/canlbp-A404PS printer/rs232/canlbp-A404PS"
typeset -r	ptype="canlbp-A404PS"
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
