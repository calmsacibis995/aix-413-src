# @(#)79	1.2  src/packages/devices/isa/pcmcia/rte/packdep.mk, pkgrspc, pkg41J, 9509A_all 2/21/95 10:06:35
#
#   COMPONENT_NAME: pkgrspc
#
#   FUNCTIONS: none
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
#
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST         += devices.isa.pcmcia.rte.pre_d

ROOT_LIBLPP_LIST        += devices.isa.pcmcia.rte.trc    \
			   devices.isa.pcmcia.rte.err    \
			   devices.isa.pcmcia.rte.config \
			   devices.isa.pcmcia.rte.unconfig

USR_ODMADD_LIST       += bus.isa.pcmcia.add%devices.isa.pcmcia.rte \
			 sm_pcmcia.add%devices.isa.pcmcia.rte

ROOT_ODMADD_LIST	+=

INFO_FILES              += devices.isa.pcmcia.rte.prereq
