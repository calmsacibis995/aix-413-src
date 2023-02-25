# @(#)54        1.7  src/packages/devices/mca/61fd/rte/packdep.mk, pkgdevtty, pkg411, GOLD410 6/28/94 17:10:06
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

USR_LIBLPP_LIST		+= devices.mca.61fd.rte.pre_d \
			   devices.mca.61fd.rte.namelist

USR_ODMADD_LIST		+= adapter.mca.64p232.add%devices.mca.61fd.rte

INFO_FILES		+= devices.mca.61fd.rte.prereq
