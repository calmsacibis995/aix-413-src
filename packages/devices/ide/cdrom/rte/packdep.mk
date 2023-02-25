# @(#)41        1.4  src/packages/devices/ide/cdrom/rte/packdep.mk, pkgdevbase, pkg41J, 9520B_all 5/17/95 17:24:44
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

USR_LIBLPP_LIST		+= devices.ide.cdrom.rte.namelist \
						devices.ide.cdrom.rte.pre_d

USR_ODMADD_LIST		+= sm_idecdrom.add%devices.ide.cdrom.rte \
			   cdrom.ide.cdrom1.add%devices.ide.cdrom.rte \
			   cdrom.ide.cdrom4X.add%devices.ide.cdrom.rte \
			   cdrom.ide.oicdrom.add%devices.ide.cdrom.rte
			  
			


INFO_FILES		+= devices.ide.cdrom.rte.prereq

ROOT_LIBLPP_LIST	+= devices.ide.cdrom.rte.trc
