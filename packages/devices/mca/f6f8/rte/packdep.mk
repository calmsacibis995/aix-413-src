# @(#)46	1.1  src/packages/devices/mca/f6f8/rte/packdep.mk, pkgdevgraphics, pkg41J, 9509A_all 2/15/95 15:08:40
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
# (C) COPYRIGHT International Business Machines Corp. 1995
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

USR_LIBLPP_LIST		+=devices.mca.f6f8.rte.pre_d

USR_ODMADD_LIST		+= mca_kma.add%devices.mca.f6f8.rte

INFO_FILES		+= devices.mca.f6f8.rte.prereq
