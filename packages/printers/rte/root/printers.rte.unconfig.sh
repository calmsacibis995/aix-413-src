#!/bin/ksh
# @(#)87        1.2  src/packages/printers/rte/root/printers.rte.unconfig.sh, pkgdevprtrte, pkg411, GOLD410 1/26/94 13:17:58
#
# COMPONENT_NAME: pkgdevprtrte
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  root/printers.rte.unconfig
#
# FUNCTION: script called from cleanup
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

set -o nounset
typeset -r		cfgfl=/var/spool/lpd/pio/@local/piocfg
typeset			hnm

# Remove hostname symbolic link and file.
hnm=$(egrep '^[ 	]*hostname[ 	]*=' $cfgfl 2>/dev/null)
hnm=$(print ${hnm#*=})
[[ -n $hnm ]] ||
{
   hnm=$(hostname 2>/dev/null)
   if [[ -n $hnm ]]
   then
      hnm=$(host "$hnm" 2>/dev/null || print - "$hnm")
   else
      hnm=$(uname -n)
   fi
   hnm=${hnm%% *}
}
rm -f $cfgfl /var/spool/lpd/pio/$hnm >/dev/null 2>&1

# Remove piobe entry from /etc/inittab file.
rmitab piobe >/dev/null 2>&1

exit 0
