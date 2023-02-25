# @(#)24        1.1  src/packages/devices/serial/tablet1/X11/packdep.mk, pkgdevx, pkg411, 9433B411a 8/16/94 11:26:05
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

USR_LIBLPP_LIST		+=
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= stablet.add%devices.serial.tablet1.X11	\
					   sm_stabmenu.add%devices.serial.tablet1.X11
ROOT_ODMADD_LIST	+=

INFO_FILES			+= devices.serial.tablet1.X11.prereq
