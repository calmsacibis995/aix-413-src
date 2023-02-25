# @(#)07        1.9  src/packages/printers/canlbp-A404PS/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:05:26
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

USR_LIBLPP_LIST		+= printers.canlbp-A404PS.rte.pre_d \
			   printers.canlbp-A404PS.rte.namelist \
			   printers.canlbp-A404PS.rte.rm_inv

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.canlbp-A404PS.diag.add%printers.canlbp-A404PS.rte \
			   printer.canlbp-A404PS.add%printers.canlbp-A404PS.rte

INFO_FILES		+= printers.canlbp-A404PS.rte.prereq
