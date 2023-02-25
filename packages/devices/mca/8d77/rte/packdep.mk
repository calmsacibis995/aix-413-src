# @(#)96        1.9  src/packages/devices/mca/8d77/rte/packdep.mk, pkgdevbase, pkg411, GOLD410 2/21/94 %U
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
#

USR_LIBLPP_LIST		+= devices.mca.8d77.rte.namelist  \
						devices.mca.8d77.rte.pre_d

ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= adapter.mca.hscsi.add%devices.mca.8d77.rte \
			   adapter.sio.hscsi.add%devices.mca.8d77.rte
ROOT_ODMADD_LIST	+=

INFO_FILES		+= devices.mca.8d77.rte.prereq
