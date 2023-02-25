# @(#)29	1.2  src/packages/devices/pcmcia/a4001e00/rte/packdep.mk, pkgrspc, pkg411, 9437A411a 9/8/94 12:23:18
#
#   COMPONENT_NAME: pkgrspc
#
#   FUNCTIONS: none
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
#
USR_LIBLPP_LIST		+= devices.pcmcia.a4001e00.rte.pre_d

USR_ODMADD_LIST		+= adapter.pcmcia.a4001e00.add%devices.pcmcia.a4001e00.rte

INFO_FILES		+= devices.pcmcia.a4001e00.rte.prereq
