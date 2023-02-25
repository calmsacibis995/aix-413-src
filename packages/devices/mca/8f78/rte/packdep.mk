# @(#)96        1.8  src/packages/devices/mca/8f78/rte/packdep.mk, pkgdevbase, pkg411, GOLD410 2/21/94 15:45:37
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

USR_LIBLPP_LIST		+=  devices.mca.8f78.rte.namelist \
						devices.mca.8f78.rte.rm_inv   \
						devices.mca.8f78.rte.pre_d

ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= adapter.mca.serdasda.add%devices.mca.8f78.rte \
			   adapter.serdasda.serdasdc.add%devices.mca.8f78.rte \
			   disk.serdasdc.serdasdd.add%devices.mca.8f78.rte \
			   disk.serdasdc.serdasdd2.add%devices.mca.8f78.rte \
			   disk.serdasdc.serdasdd4.add%devices.mca.8f78.rte \
			   disk.serdasdc.2000mb.add%devices.mca.8f78.rte 

INFO_FILES		+= devices.mca.8f78.rte.prereq
