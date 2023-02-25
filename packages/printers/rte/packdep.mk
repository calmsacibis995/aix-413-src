# @(#)86        1.7  src/packages/printers/rte/packdep.mk, pkgdevprtrte, pkg411, GOLD410 2/7/94 18:08:45
#
# COMPONENT_NAME: pkgdevprtrte
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
INFO_FILES		+= printers.rte.insize

USR_LIBLPP_LIST		+= printers.rte.namelist \
			   printers.rte.rm_inv

ROOT_LIBLPP_LIST	+= printers.rte.config \
			   printers.rte.unconfig \
			   printers.rte.namelist \
			   printers.rte.rm_inv

USR_ODMADD_LIST		+= cmdpios.add%printers.rte
