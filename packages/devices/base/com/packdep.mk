# @(#)53        1.3  src/packages/devices/base/com/packdep.mk, pkgdevbase, pkg411, GOLD410 6/3/94 17:38:48
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

USR_LIBLPP_LIST		+=  devices.base.com.namelist \
			    devices.base.com.pre_d

ROOT_LIBLPP_LIST	+=	

USR_ODMADD_LIST		+= memory.sys.memory.add%devices.base.com \
			   memory.sys.simm.add%devices.base.com \
			   processor.sys.proc1.add%devices.base.com  \
			   cpucard.sys.cpucard1.add%devices.base.com  \
                           sysunit.sys.sysunit.add%devices.base.com \
			   memory.sys.L2cache.add%devices.base.com


ROOT_ODMADD_LIST        += cfgrule.sys.add%devices.base.com


