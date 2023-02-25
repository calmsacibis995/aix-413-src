#! /bin/ksh
# @(#)48	1.1  src/packages/devices/mca/f6f8/rte/usr/devices.mca.f6f8.rte.pre_d.sh, pkgdevgraphics, pkg41J, 9509A_all 2/15/95 15:08:46
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: Determine whether Customized device entry exists
#              for a given device package option
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
#      0    if deinstall of package option IS permitted
#      1    if deinstall of package option IS NOT permitted
#

uniquetypes="adapter/mca/kma adapter/kma/keyboard adapter/kma/mouse"

for type in $uniquetypes
do
 rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
 if [ -n "$rc" ]
 then
  exit 1
 fi
done
exit 0

