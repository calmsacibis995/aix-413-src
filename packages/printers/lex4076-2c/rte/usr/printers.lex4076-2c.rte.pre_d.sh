#!/usr/bin/ksh
# @(#)69        1.1  src/packages/printers/lex4076-2c/rte/usr/printers.lex4076-2c.rte.pre_d.sh, pkgdevprint, pkg41J, 9512A_all 3/15/95 19:15:06
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
typeset -r	uniquetypes="printer/parallel/lex4076-2c printer/rs232/lex4076-2c printer/rs422/lex4076-2c"
typeset -r	ptype="lex4076-2c"
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

