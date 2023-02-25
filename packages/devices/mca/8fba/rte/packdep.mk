# @(#)38        1.8  src/packages/devices/mca/8fba/rte/packdep.mk, pkgdevbase, pkg411, GOLD410 2/21/94 15:46:11
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

USR_LIBLPP_LIST		+= devices.mca.8fba.rte.namelist \
						devices.mca.8fba.rte.pre_d

ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= adapter.mca.8fba.add%devices.mca.8fba.rte

INFO_FILES		+= devices.mca.8fba.rte.prereq
