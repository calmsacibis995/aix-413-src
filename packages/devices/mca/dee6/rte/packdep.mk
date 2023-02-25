# @(#)20        1.9  src/packages/devices/mca/dee6/rte/packdep.mk, pkgdevbase, pkg411, GOLD410 2/24/94 17:29:09
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
#		config scripts or prereq files would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.

USR_LIBLPP_LIST		+= devices.mca.dee6.rte.pre_d \
						devices.mca.dee6.rte.namelist

ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+=  adapter.mca.sio_1.add%devices.mca.dee6.rte

INFO_FILES		+= devices.mca.dee6.rte.prereq
