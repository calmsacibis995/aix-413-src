# @(#)25        1.4  src/packages/bos/INed/packdep.mk, pkgined, pkg411, GOLD410 4/9/94 16:05:06
#
# COMPONENT_NAME: pkgined
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts or prereq files would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

INFO_FILES		+= bos.INed.insize 	\
			   bos.INed.supersede

USR_LIBLPP_LIST		+= lpp.README bos.INed.namelist

USR_ODMADD_LIST		+=

ROOT_LIBLPP_LIST	+= bos.INed.namelist

ROOT_ODMADD_LIST	+=

