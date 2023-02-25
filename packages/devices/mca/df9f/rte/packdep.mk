# @(#)50        1.7  src/packages/devices/mca/df9f/rte/packdep.mk, pkgdevbase, pkg411, GOLD410 2/21/94 15:47:39
#
# COMPONENT_NAME: pkgdevbase
#
# FUNCTIONS: packaging definition
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

USR_LIBLPP_LIST		+= devices.mca.df9f.rte.namelist \
						devices.mca.df9f.rte.pre_d

ROOT_LIBLPP_LIST	+= 

USR_ODMADD_LIST		+= disk.mca.badisk.add%devices.mca.df9f.rte
ROOT_ODMADD_LIST	+=

INFO_FILES		+= devices.mca.df9f.rte.prereq
