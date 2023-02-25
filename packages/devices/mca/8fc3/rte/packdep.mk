# @(#)19        1.7  src/packages/devices/mca/8fc3/rte/packdep.mk, pkgdevcommo, pkg411, GOLD410 7/5/94 16:32:58
#
# COMPONENT_NAME: pkgdevcommo
#
# FUNCTIONS: packaging definitions
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

USR_LIBLPP_LIST		+=

#USR_LIBLPP_LIST	+= devices.mca.8fc3.rte.pre_i \
#			   devices.mca.8fc3.rte.post_i
#
#ROOT_LIBLPP_LIST	+= devices.mca.8fc3.rte.trc \
#			   devices.mca.8fc3.rte.err \
#			   devices.mca.8fc3.rte.pre_i \
#			   devices.mca.8fc3.rte.post_i \
#			   devices.mca.8fc3.rte.config \
#			   devices.mca.8fc3.rte.unconfig \
#			   devices.mca.8fc3.rte.unpost_i

USR_ODMADD_LIST		+= escon.usr.add%devices.mca.8fc3.rte \
			   sm_escon.usr.add%devices.mca.8fc3.rte

ROOT_ODMADD_LIST	+= escon.root.add%devices.mca.8fc3.rte \
			   sm_escon.root.add%devices.mca.8fc3.rte

INFO_FILES		+= devices.mca.8fc3.rte.prereq
