#! /bin/ksh
# @(#)46        1.1  src/packages/devices/isa/cxia/rte/usr/devices.isa.cxia.rte.pre_d.sh, pkgdevtty, pkg411, GOLD410 6/9/94 16:32:42
#
#   COMPONENT_NAME: pkgdevtty
#
#   FUNCTIONS: Determine whether Customized device entry exists
#              for a given device package option
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
#      0    if deinstall of package option IS permitted
#      1    if deinstall of package option IS NOT permitted
#
uniquetypes="adapter/isa/cxia"

for type in $uniquetypes
do
 rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
 if [ -n "$rc" ]
 then
  exit 1
 fi
done
exit 0

