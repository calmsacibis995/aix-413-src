# @(#)07	1.2  src/packages/devices/pci/3353c388/rte/packdep.mk, pkgrspc, pkg411, 9438A411a 9/19/94 11:40:29
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

USR_LIBLPP_LIST		+= devices.pci.3353c388.rte.pre_d
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= 3353c388.add%devices.pci.3353c388.rte
ROOT_ODMADD_LIST	+=

INFO_FILES		+= devices.pci.3353c388.rte.prereq
