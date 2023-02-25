# @(#)68        1.7  src/packages/devices/mca/8f70/diag/packdep.mk, pkgdevdiag, pkg411, GOLD410 3/10/94 12:57:52
#
# COMPONENT_NAME: pkgdevdiag
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
# *_ODMADD_LIST	contains the list of odmadd files that must be added
#		to the respective liblpp.a files.  The variable may be
#		left blank if there are no odmadd files for this package.
#

USR_LIBLPP_LIST		+= devices.mca.8f70.diag.namelist \
			   devices.mca.8f70.diag.rm_inv
ROOT_LIBLPP_LIST        +=

USR_ODMADD_LIST	      += diag.8f70.add%devices.mca.8f70.diag \
			 darticdd.add%devices.mca.8f70.diag
ROOT_ODMADD_LIST     +=

INFO_FILES		+= devices.mca.8f70.diag.prereq
