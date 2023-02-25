# @(#)11        1.5  src/packages/X11/motif/mwm/packdep.mk, pkgx, pkg411, GOLD410 5/13/94 09:54:51
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

USR_LIBLPP_LIST		+= X11.motif.mwm.post_i	\
					   X11.motif.mwm.namelist	\
					   X11.motif.mwm.rm_inv		\
					   X11.motif.mwm.cfgfiles
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+=
ROOT_ODMADD_LIST	+=

INFO_FILES			+= X11.motif.mwm.prereq
