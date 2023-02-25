# @(#)93        1.11  src/packages/printers/bull970/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 4/28/94 07:51:25
#
# COMPONENT_NAME: pkgdevprint
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= printers.bull970.rte.pre_d \
			   printers.bull970.rte.namelist \
			   printers.bull970.rte.rm_inv
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.bull970.diag.add%printers.bull970.rte \
			   printer.bull970.add%printers.bull970.rte
ROOT_ODMADD_LIST	+=

INFO_FILES      += printers.bull970.rte.prereq
