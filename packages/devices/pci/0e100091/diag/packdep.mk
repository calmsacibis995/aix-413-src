# @(#)17	1.1  src/packages/devices/pci/0e100091/diag/packdep.mk, pkgdevdiag, pkg41J, 9515A_all 4/3/95 16:39:11
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
# (C) COPYRIGHT International Business Machines Corp. 1995
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
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= diag.0e100091.add%devices.pci.0e100091.diag
ROOT_ODMADD_LIST	+= 
INFO_FILES              += devices.pci.0e100091.diag.prereq
