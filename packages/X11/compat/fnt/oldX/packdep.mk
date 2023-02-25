# @(#)55	1.8  src/packages/X11/compat/fnt/oldX/packdep.mk, pkgx, pkg41B, 412_41B_sync 1/16/95 17:04:18
#
#   COMPONENT_NAME: pkgx
#
# FUNCTIONS: packaging definitions
#
#   ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#   (C) COPYRIGHT International Business Machines Corp. 1993,1995
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

USR_LIBLPP_LIST		+= X11.compat.fnt.oldX.post_i	\
					   X11.compat.fnt.oldX.unpre_i	\
					   X11.compat.fnt.oldX.namelist	\
					   X11.compat.fnt.oldX.rm_inv

USR_LIBLPP_UPDT_LIST    += X11.compat.fnt.oldX.post_u

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+=
ROOT_ODMADD_LIST	+=

INFO_FILES			+= X11.compat.fnt.oldX.prereq \
				   X11.compat.fnt.oldX.supersede
