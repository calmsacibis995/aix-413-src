# @(#)10        1.12  src/packages/devices/sys/slc/rte/packdep.mk, pkgdevcommo, pkg411, GOLD410 7/6/94 08:07:31
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

USR_LIBLPP_LIST         += devices.sys.slc.rte.pre_d \
			   devices.sys.slc.rte.namelist \
			   devices.sys.slc.rte.rm_inv  \
			   devices.sys.slc.rte.cfginfo  \

ROOT_LIBLPP_LIST        += devices.sys.slc.rte.trc      \
			   devices.sys.slc.rte.err

USR_ODMADD_LIST		+= ops.add%devices.sys.slc.rte \
			   sm_sol.add%devices.sys.slc.rte \
			   sol.add%devices.sys.slc.rte

ROOT_ODMADD_LIST	+= ops_rules.add%devices.sys.slc.rte
