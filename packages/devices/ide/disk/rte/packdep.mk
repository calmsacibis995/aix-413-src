# @(#)50        1.5  src/packages/devices/ide/disk/rte/packdep.mk, pkgdevbase, pkg41J, 9520A_all 5/16/95 15:40:20
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

USR_LIBLPP_LIST		+= devices.ide.disk.rte.namelist \
						devices.ide.disk.rte.pre_d

ROOT_LIBLPP_LIST	+= devices.ide.disk.rte.err \
			   devices.ide.disk.rte.codepoint \
			   devices.ide.disk.rte.trc

USR_ODMADD_LIST		+= sm_idedisk.add%devices.ide.disk.rte \
                           disk.ide.1080mbDPEA.add%devices.ide.disk.rte \
                           disk.ide.730mb.add%devices.ide.disk.rte \
                           disk.ide.730mbWD.add%devices.ide.disk.rte \
                           disk.ide.720mb.add%devices.ide.disk.rte \
                           disk.ide.540mb.add%devices.ide.disk.rte \
                           disk.ide.540mbDALA.add%devices.ide.disk.rte \
                           disk.ide.540mbWD.add%devices.ide.disk.rte \
                           disk.ide.oidisk.add%devices.ide.disk.rte

INFO_FILES		+= devices.ide.disk.rte.prereq

