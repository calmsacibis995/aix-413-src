# @(#)40        1.1  src/packages/devices/isa_sio/km/rte/packdep.mk, pkgrspc, pkg411, GOLD410 6/28/94 07:33:12
#
# COMPONENT_NAME: pkgrspc
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST	  +=  devices.isa_sio.km.rte.pre_d

ROOT_LIBLPP_LIST  +=	

USR_ODMADD_LIST   += adapter.isa_sio.isa_mouse.add%devices.isa_sio.km.rte \
		     adapter.isa_sio.isa_keyboard.add%devices.isa_sio.km.rte 

INFO_FILES        += devices.isa_sio.km.rte.prereq


