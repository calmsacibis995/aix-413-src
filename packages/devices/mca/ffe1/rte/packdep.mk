# @(#)01        1.12  src/packages/devices/mca/ffe1/rte/packdep.mk, pkgdevtty, pkg41J, 9520A_all 4/27/95 16:14:54
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
OBJCLASSDB               = ${ODE_TOOLS}/usr/lib/uniqueobjclassdb

USR_LIBLPP_LIST		+= devices.mca.ffe1.rte.namelist \
			   devices.mca.ffe1.rte.pre_d \
			   devices.mca.ffe1.rte.158660.odmdel

ROOT_LIBLPP_LIST	+= devices.mca.ffe1.rte.err \
			   devices.mca.ffe1.rte.trc

INFO_FILES		+= devices.mca.ffe1.rte.prereq
