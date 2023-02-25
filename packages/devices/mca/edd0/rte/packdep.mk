# @(#)65        1.6  src/packages/devices/mca/edd0/rte/packdep.mk, pkgdevtty, pkg411, GOLD410 6/6/94 15:38:24
#
# COMPONENT_NAME: pkgdevtty
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

USR_LIBLPP_LIST		+=devices.mca.edd0.rte.pre_d

USR_ODMADD_LIST		+= adapter.mca.8p232.add%devices.mca.edd0.rte

INFO_FILES		+= devices.mca.edd0.rte.prereq
