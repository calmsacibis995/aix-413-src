#! /bin/ksh
# @(#)70        1.2 4/20/94 src/packages/devices/sys/sgabus/rte/usr/devices.sys.sgabus.rte.pre_d.sh, pkgdevgraphics, pkg411, GOLD410 10:16:54
#
#   COMPONENT_NAME: pkgdevgraphics
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
uniquetypes="bus/sys/sgabus"
for type in $uniquetypes
do
 rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
 if [ -n "$rc" ]
 then
  exit 1
 fi
done
exit 0

