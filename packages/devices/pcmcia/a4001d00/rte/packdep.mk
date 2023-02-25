# @(#)98	1.1  src/packages/devices/pcmcia/a4001d00/rte/packdep.mk, pkgrspc, pkg411, 9437A411a 9/8/94 14:45:29
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
USR_LIBLPP_LIST		+= devices.pcmcia.a4001d00.rte.pre_d

USR_ODMADD_LIST		+= adapter.pcmcia.a4001d00.add%devices.pcmcia.a4001d00.rte

INFO_FILES		+= devices.pcmcia.a4001d00.rte.prereq
