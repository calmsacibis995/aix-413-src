# @(#)60        1.7  src/packages/printers/canlbp/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:04:06
#
# COMPONENT_NAME: pkgdevprint
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

USR_LIBLPP_LIST		+= printers.canlbp.rte.pre_d \
			   printers.canlbp.rte.namelist \
			   printers.canlbp.rte.rm_inv
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.canlbp.diag.add%printers.canlbp.rte \
			   printer.canlbp.add%printers.canlbp.rte
ROOT_ODMADD_LIST	+=

INFO_FILES		+= printers.canlbp.rte.prereq
