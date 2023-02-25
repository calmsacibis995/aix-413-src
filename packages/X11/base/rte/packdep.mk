# @(#)95        1.9  src/packages/X11/base/rte/packdep.mk, pkgx, pkg411, GOLD410 6/13/94 09:24:07
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

USR_LIBLPP_LIST		+= X11.base.rte.post_i		\
			   X11.base.rte.namelist	\
			   X11.base.rte.rm_inv		\
			   X11.base.rte.cfgfiles	\
			   X11.base.rte.41cfgfiles

ROOT_LIBLPP_LIST	+= X11.base.rte.post_i		\
			   X11.base.rte.unpost_i

USR_ODMADD_LIST		+= X11input_base.add%X11.base.rte
ROOT_ODMADD_LIST	+=

INFO_FILES		+= X11.base.rte.prereq		\
			   X11.base.rte.supersede
