#! /bin/ksh
# @(#)03        1.2  src/packages/devices/sio/sa/rte/usr/devices.sio.sa.rte.pre_d.sh, pkgdevtty, pkg411, GOLD410 4/21/94 15:47:16
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
uniquetypes="adapter/sio/s1a adapter/sio/s1a_1 adapter/sio/s1a_3
             adapter/sio/s2a adapter/sio/s2a_1 adapter/sio/s2a_3
             adapter/sio/s3a_3"

for type in $uniquetypes
do
 rc=$(ODMDIR=/etc/objrepos odmget -q PdDvLn=$type CuDv)
 if [ -n "$rc" ]
 then
  exit 1
 fi
done
exit 0

