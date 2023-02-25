# @(#)04        1.10 6/2/94 src/packages/devices/sio/ktma/rte/packdep.mk, pkgdevgraphics, pkg411, GOLD410 15:08:42
#
# COMPONENT_NAME: pkgdevgraphics
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

USR_LIBLPP_LIST		+= devices.sio.ktma.rte.pre_d

USR_ODMADD_LIST		+= mouse.add%devices.sio.ktma.rte \
			   kts.add%devices.sio.ktma.rte \
			   tablet.add%devices.sio.ktma.rte \
			   kbd.add%devices.sio.ktma.rte

INFO_FILES              += devices.sio.ktma.rte.prereq
