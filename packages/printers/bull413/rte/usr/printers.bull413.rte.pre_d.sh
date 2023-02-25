#!/usr/bin/ksh
# @(#)35 1.1 src/packages/printers/bull413/rte/usr/printers.bull413.rte.pre_d.sh, pkgdevprint, pkg411, GOLD410 4/28/94 08:33:02
#
# COMPONENT_NAME: pkgdevprint
#
# FUNCTIONS: Determine whether Customized device entry exists
#              for a given printer package option
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#
# Return:  
#    		0    if deinstall of package option IS permitted
#    		1    if deinstall of package option IS NOT permitted
#
typeset -r	uniquetypes="printer/parallel/bull413 printer/rs232/bull413 printer/rs422/bull413"
typeset -r	ptype="bull413"
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

