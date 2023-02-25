# @(#)29        1.3  src/packages/X11/loc/iw_IL/base/lib/packdep.mk, pkgx, pkg411, GOLD410 5/27/94 11:55:38
#
# COMPONENT_NAME: pkgx
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
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

USR_LIBLPP_LIST		+= X11.loc.iw_IL.base.lib.cfgfiles
ROOT_LIBLPP_LIST	+=

USR_ODMADD_OPTIONS	+=
ROOT_ODMADD_OPTIONS	+=

USR_ODMADD_FILES	+=
ROOT_ODMADD_FILES	+=

INFO_FILES			+= X11.loc.iw_IL.base.lib.prereq
