# @(#)34        1.5  src/packages/devices/mca/8f9a/diag/packdep.mk, pkgneptune, pkg411, GOLD410 6/24/94 11:59:22
#
# COMPONENT_NAME: pkgdevdiag
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

USR_LIBLPP_LIST		+= devices.mca.8f9a.diag.namelist \
			   devices.mca.8f9a.diag.rm_inv

USR_ODMADD_LIST		+= diag.8f9a.add%devices.mca.8f9a.diag

INFO_FILES		+= devices.mca.8f9a.diag.prereq
