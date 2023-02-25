# @(#)38        1.1  src/packages/bos/dev/sys/mca/rte/packdep.mk, pkgdevbase, pkg410, theme.f90007 9/23/93 11:21:23
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

USR_LIBLPP_LIST		+=  devices.sys.mca.rte.namelist \
 						devices.sys.mca.rte.pre_d
ROOT_LIBLPP_LIST    +=

USR_ODMADD_LIST		+= bus.sys.mca.add%devices.sys.mca.rte \
			   ioplanar.sys.ioplanar.add%devices.sys.mca.rte \
			   ioplanar.sys.ioplanar_1.add%devices.sys.mca.rte \
 			   ioplanar.sys.ioplanar_2.add%devices.sys.mca.rte
