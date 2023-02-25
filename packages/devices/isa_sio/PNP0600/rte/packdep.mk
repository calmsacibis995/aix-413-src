# @(#)65        1.2  src/packages/devices/isa_sio/PNP0600/rte/packdep.mk, pkgdevbase, pkg41J, 9510B 2/21/95 14:34:03
#
# COMPONENT_NAME: pkgdevbase
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
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

USR_LIBLPP_LIST		+= devices.isa_sio.PNP0600.rte.namelist \
					devices.isa_sio.PNP0600.rte.pre_d

ROOT_LIBLPP_LIST	+= devices.isa_sio.PNP0600.rte.err \
			   devices.isa_sio.PNP0600.rte.trc

USR_ODMADD_LIST		+= sm_ide.add%devices.isa_sio.PNP0600.rte \
			   adapter.isa_sio.ide.add%devices.isa_sio.PNP0600.rte 

INFO_FILES		+= devices.isa_sio.PNP0600.rte.prereq

