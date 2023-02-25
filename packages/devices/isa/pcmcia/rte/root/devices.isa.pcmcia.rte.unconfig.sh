#!/usr/bin/ksh
# @(#)50	1.1  src/packages/devices/isa/pcmcia/rte/root/devices.isa.pcmcia.rte.unconfig.sh, pkgrspc, pkg41J, 9509A_all 2/21/95 11:19:33
#
#   COMPONENT_NAME: PKGRSPC
#
#   FUNCTIONS: none
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

/usr/sbin/rmitab acfgd > /dev/null 2>&1
exit $?
