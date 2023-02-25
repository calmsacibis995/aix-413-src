# @(#)40        1.1  src/packages/bos/adt/graphics/packdep.mk, pkgdevgraphics, pkg411, GOLD410 4/21/94 14:25:55
#
# COMPONENT_NAME: pkgdevgraphics
#
# FUNCTIONS: packaging definition
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

USR_LIBLPP_LIST		+= bos.adt.graphics.namelist bos.adt.graphics.rm_inv

USR_ODMADD_LIST		+=
