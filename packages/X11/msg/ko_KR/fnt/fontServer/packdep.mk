# @(#)02        1.2  src/packages/X11/msg/ko_KR/fnt/fontServer/packdep.mk, pkgx, pkg411, GOLD410 4/28/94 17:55:26
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

USR_LIBLPP_LIST		+= X11.msg.ko_KR.fnt.fontServer.namelist	\
					   X11.msg.ko_KR.fnt.fontServer.rm_inv
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+=
ROOT_ODMADD_LIST	+=


INFO_FILES			+= X11.msg.ko_KR.fnt.fontServer.prereq
