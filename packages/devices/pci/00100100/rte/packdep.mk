# @(#)66        1.3  src/packages/devices/pci/00100100/rte/packdep.mk, pkgrspc, pkg41J, 9525F_all 6/21/95 16:23:01
#
# COMPONENT_NAME: pkgrspc
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994,1995
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

USR_LIBLPP_LIST	  +=  devices.pci.00100100.rte.pre_d \
											devices.pci.00100100.rte.pre_i

ROOT_LIBLPP_LIST  +=	

USR_ODMADD_LIST   +=  adapter.pci.ncr810.add%devices.pci.00100100.rte 

INFO_FILES        += 


