#! /bin/ksh
# @(#)87	1.1  src/packages/devices/pci/14101c00/rte/usr/devices.pci.14101c00.rte.pre_d.sh, pkgpwrmgt, pkg412, 9447C 11/22/94 06:08:15
#
#   COMPONENT_NAME: pkgpwrmgt
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
odmdelete -q PdDvLn=adapter/pci/14101c00 -o CuDv > /dev/null 2>&1
exit 0
