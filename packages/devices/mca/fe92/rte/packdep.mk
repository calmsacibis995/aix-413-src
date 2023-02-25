# @(#)85	1.11  src/packages/devices/mca/fe92/rte/packdep.mk, pkgdevcommo, pkg411, 9439C411a 9/29/94 16:24:24
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

USR_LIBLPP_LIST		+= devices.mca.fe92.rte.pre_d \
			   devices.mca.fe92.rte.rm_inv \
			   devices.mca.fe92.rte.namelist

ROOT_LIBLPP_LIST	+= devices.mca.fe92.rte.trc \

USR_ODMADD_LIST		+= cat.add%devices.mca.fe92.rte

ROOT_ODMADD_LIST	+=

INFO_FILES		+= devices.mca.fe92.rte.prereq \
			   devices.mca.fe92.rte.supersede

