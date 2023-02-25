# @(#)02	1.2  src/packages/printers/epsonLQ1600K_CN/rte/usr/printers.epsonLQ1600K_CN.rte.pre_d.sh, pkgdevprint, pkg41J, 9507B 2/1/95 11:02:24
#
#   COMPONENT_NAME: ils-zh_CN
#
#   FUNCTIONS: none
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
#
# Return:  
#    		0    if deinstall of package option IS permitted
#    		1    if deinstall of package option IS NOT permitted
#
typeset -r	uniquetypes="printer/parallel/epsonLQ1600K printer/rs232/epsonLQ1600K"
typeset -r	ptype="epsonLQ1600K"
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

