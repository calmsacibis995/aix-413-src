#! /bin/ksh
# @(#)64        1.2 4/20/94 src/packages/devices/mca/8fbc/rte/usr/devices.mca.8fbc.rte.pre_d.sh, pkgdevgraphics, pkg411, GOLD410 11:04:42
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
uniquetypes="adapter/mca/rby adapter/mca.rby1/rbygpss adapter/mca.rby2/rbyrss
             adapter/mca.rby3/rbyvoo"
for type in $uniquetypes
do
 rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
 if [ -n "$rc" ]
 then
  exit 1
 fi
done
exit 0

