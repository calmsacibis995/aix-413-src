# @(#)70        1.7  src/packages/devices/mca/eff0/rte/packdep.mk, pkgdevcommo, pkg411, GOLD410 4/6/94 12:25:34
#
# COMPONENT_NAME: pkgdevcommo
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
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

USR_LIBLPP_LIST		+= devices.mca.eff0.rte.namelist \
			   devices.mca.eff0.rte.rm_inv \
			   devices.mca.eff0.rte.pre_d
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= ampx.add%devices.mca.eff0.rte \
			   sm_ampx.add%devices.mca.eff0.rte

ROOT_ODMADD_LIST	+=

