# @(#)81	1.1  src/packages/devices/pcmcia/tokenring/com/packdep.mk, pkgrspc, pkg411, 9437A411a 9/8/94 12:33:24
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
#ROOT_LIBLPP_LIST	+= devices.pcmcia.tokenring.com.trc	\
#			   devices.pcmcia.tokenring.com.err

USR_ODMADD_LIST		+= sm_pcmciatok.add%devices.pcmcia.tokenring.com

INFO_FILES		+= devices.pcmcia.tokenring.com.prereq
