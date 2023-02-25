# @(#)01        1.7 6/28/94 src/packages/devices/mca/8787/rte/packdep.mk, pkgdevgraphics, pkg411, GOLD410 17:17:28
#
# COMPONENT_NAME: pkgdevgraphics
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

USR_LIBLPP_LIST		+= devices.mca.8787.rte.pre_d \
			   devices.mca.8787.rte.cfginfo \
			   devices.mca.8787.rte.namelist

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= msla.add%devices.mca.8787.rte \
			   psla.add%devices.mca.8787.rte \
			   ssla.add%devices.mca.8787.rte \
			   sm_msla.add%devices.mca.8787.rte
ROOT_ODMADD_LIST	+=

INFO_FILES		+= devices.mca.8787.rte.prereq
