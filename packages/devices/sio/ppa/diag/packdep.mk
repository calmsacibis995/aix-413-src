# @(#)52	1.8  src/packages/devices/sio/ppa/diag/packdep.mk, pkgdevdiag, pkg411, GOLD410 3/10/94 11:18:42
#
# COMPONENT_NAME: pkgdevdiag
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

USR_LIBLPP_LIST		+= devices.sio.ppa.diag.namelist
ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= diag.ppa.add%devices.sio.ppa.diag \
			   printer.opp.diag.add%devices.sio.ppa.diag
ROOT_ODMADD_LIST	+= 
INFO_FILES              += devices.sio.ppa.diag.prereq
