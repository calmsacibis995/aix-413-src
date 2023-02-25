#! /bin/ksh
# @(#)68	1.4  src/packages/devices/sio/ktma/rte/usr/devices.sio.ktma.rte.pre_d.sh, pkgdevgraphics, pkg41J, 9509A_all 2/15/95 15:09:29
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: Determine whether Customized device entry exists
#              for a given device package option
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Return:  
#      0    if deinstall of package option IS permitted
#      1    if deinstall of package option IS NOT permitted
#

uniquetypes="adapter/sio/keyboard_2 adapter/sio/keyboard adapter/sio/mouse
             adapter/sio/tablet_2"

for type in $uniquetypes
do
 rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
 if [ -n "$rc" ]
 then
  exit 1
 fi
done
exit 0

