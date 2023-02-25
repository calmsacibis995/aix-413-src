# @(#)14        1.4  src/packages/devices/mca/fed9/rte/packdep.mk, pkgdevbase, pkg411, GOLD410 1/26/94 08:52:06
#
# COMPONENT_NAME: pkgdevbase
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

USR_ODMADD_LIST		+= adapter.mca.sio_3.add%devices.mca.fed9.rte

INFO_FILES		+= devices.mca.fed9.rte.prereq
