# @(#)38	1.2  src/packages/devices/pci/14101c00/rte/packdep.mk, pkgpwrmgt, pkg412, 9447C 11/22/94 06:10:32
#
# COMPONENT_NAME: pkgpwrmgt
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
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

USR_LIBLPP_LIST	  +=  devices.pci.14101c00.rte.pre_d

ROOT_LIBLPP_LIST  +=

USR_ODMADD_LIST   +=  adapter.pci.14101c00.add%devices.pci.14101c00.rte

INFO_FILES        +=  devices.pci.14101c00.rte.prereq
