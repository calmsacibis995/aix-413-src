# @(#)08        1.3  src/packages/devices/isa/ethernet/rte/packdep.mk, pkgrspc, pkg411, GOLD410 7/5/94 14:18:59
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

USR_LIBLPP_LIST         += devices.isa.ethernet.rte.pre_d 

ROOT_LIBLPP_LIST        += devices.isa.ethernet.rte.trc    \
			   devices.isa.ethernet.rte.err

USR_ODMADD_LIST       += isaent.add%devices.isa.ethernet.rte \
			 sm_isaent.add%devices.isa.ethernet.rte
ROOT_ODMADD_LIST	+=

INFO_FILES              += devices.isa.ethernet.rte.prereq
