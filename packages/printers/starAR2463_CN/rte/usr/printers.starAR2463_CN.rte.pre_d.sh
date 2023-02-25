# @(#)17	1.2  src/packages/printers/starAR2463_CN/rte/usr/printers.starAR2463_CN.rte.pre_d.sh, pkgdevprint, pkg41J, 9507B 2/1/95 11:01:51
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
typeset -r	uniquetypes="printer/parallel/starAR2463 printer/rs232/starAR2463"
typeset -r	ptype="starAR2463"
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

