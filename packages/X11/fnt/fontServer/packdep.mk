# @(#)34        1.6  src/packages/X11/fnt/fontServer/packdep.mk, pkgx, pkg411, GOLD410 5/13/94 08:59:29
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

USR_LIBLPP_LIST		+= X11.fnt.fontServer.post_i	\
					   X11.fnt.fontServer.namelist	\
					   X11.fnt.fontServer.rm_inv	\
					   X11.fnt.fontServer.cfgfiles	\
					   X11.fnt.fontServer.41cfgfiles
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+=
ROOT_ODMADD_LIST	+=

INFO_FILES			+= X11.fnt.fontServer.prereq
