# @(#)09	1.5  src/packages/X11/fnt/ibm1046_T1/packdep.mk, pkgx, pkg41B, 412_41B_sync 1/16/95 17:03:40
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

USR_LIBLPP_LIST		+= X11.fnt.ibm1046_T1.post_i	\
					   X11.fnt.ibm1046_T1.unpost_i	\
					   X11.fnt.ibm1046_T1.unpre_i

USR_LIBLPP_UPDT_LIST    += X11.fnt.ibm1046_T1.post_u

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+=
ROOT_ODMADD_LIST	+=

INFO_FILES			+= X11.fnt.ibm1046_T1.prereq
