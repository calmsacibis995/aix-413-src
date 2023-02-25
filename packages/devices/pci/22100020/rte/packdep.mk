# @(#)75	1.1  src/packages/devices/pci/22100020/rte/packdep.mk, pkgrspc, pkg41J, 9507B 1/30/95 14:01:05
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

USR_LIBLPP_LIST         += devices.pci.22100020.rte.pre_d 

ROOT_LIBLPP_LIST        += devices.pci.22100020.rte.trc    \
			   devices.pci.22100020.rte.err

USR_ODMADD_LIST       += kent.add%devices.pci.22100020.rte

ROOT_ODMADD_LIST	+=

INFO_FILES              += devices.pci.22100020.rte.prereq
