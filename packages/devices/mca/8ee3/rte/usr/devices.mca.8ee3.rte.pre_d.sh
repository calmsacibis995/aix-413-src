#! /bin/ksh
# @(#)61        1.3 5/3/94 src/packages/devices/mca/8ee3/rte/usr/devices.mca.8ee3.rte.pre_d.sh, pkgdevgraphics, pkg411, GOLD410 18:44:50
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
uniquetypes="adapter/mca/ppr adapter/optcard/pgr adapter/optcard/pge \ 
            adapter/optcard/pgt adapter/optcard/pop adapter/optcard/ppc" 
for type in $uniquetypes
do
 rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
 if [ -n "$rc" ]
 then
  exit 1
 fi
done
exit 0

