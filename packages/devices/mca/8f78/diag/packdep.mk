# @(#)69        1.7  src/packages/devices/mca/8f78/diag/packdep.mk, pkgdevdiag, pkg411, GOLD410 3/10/94 12:55:25
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

USR_LIBLPP_LIST		+= devices.mca.8f78.diag.namelist
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= diag.8f78.add%devices.mca.8f78.diag
ROOT_ODMADD_LIST	+=
INFO_FILES              += devices.mca.8f78.diag.prereq
