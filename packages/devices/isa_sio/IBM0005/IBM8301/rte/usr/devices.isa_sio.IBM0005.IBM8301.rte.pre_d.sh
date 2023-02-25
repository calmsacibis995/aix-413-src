#! /bin/ksh
#
#   COMPONENT_NAME: pkgpwrmgt
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
odmdelete -q 'PdDvLn=adapter/isa_sio/IBM0005.IBM8301' -o CuDv > /dev/null 2>&1
exit 0
