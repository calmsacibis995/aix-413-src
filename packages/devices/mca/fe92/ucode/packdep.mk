# @(#)96	1.4  src/packages/devices/mca/fe92/ucode/packdep.mk, pkgdevcommo, pkg411, GOLD410 2/25/94 13:46:25
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

USR_LIBLPP_LIST		+= devices.mca.fe92.ucode.namelist \
			   devices.mca.fe92.ucode.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+=

ROOT_ODMADD_LIST	+=
