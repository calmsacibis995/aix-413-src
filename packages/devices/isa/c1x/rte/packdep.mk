# @(#)92    1.1  src/packages/devices/isa/c1x/rte/packdep.mk, pkgrspc, pkg411, GOLD410  5/18/94  09:15:14
#
# COMPONENT_NAME: pkgrspc
#
# FUNCTIONS: packaging definition
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

USR_LIBLPP_LIST		+= devices.isa.c1x.rte.namelist \
			   devices.isa.c1x.rte.rm_inv \
			   devices.isa.c1x.rte.pre_d
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= adapter.isa.c1x.add%devices.isa.c1x.rte \
			   sm_c1x.add%devices.isa.c1x.rte

ROOT_ODMADD_LIST	+=

