# @(#)73	1.1  src/packages/devices/pcmcia/ethernet/com/packdep.mk, pkgrspc, pkg411, 9437A411a 9/8/94 12:06:02
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
#ROOT_LIBLPP_LIST	+= devices.pcmcia.ethernet.com.trc	\
#			   devices.pcmcia.ethernet.com.err

USR_ODMADD_LIST		+= sm_pcmciaent.add%devices.pcmcia.ethernet.com

INFO_FILES		+= devices.pcmcia.ethernet.com.prereq
