# @(#)49        1.3  src/packages/devices/rs6ksmp/base/rte/packdep.mk, pkgdevbase, pkg41J, 9511A_all 3/14/95 16:31:55
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
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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

USR_LIBLPP_LIST	  +=  devices.rs6ksmp.base.rte.pre_d

ROOT_LIBLPP_LIST  +=	

USR_ODMADD_LIST   += sys.node.sys_p.add%devices.rs6ksmp.base.rte \
		   planar.sys.sysplanar_p.add%devices.rs6ksmp.base.rte \
		   ioplanar.sys.iodplanar1.add%devices.rs6ksmp.base.rte \
		   ioplanar.planar.mcaplanar1.add%devices.rs6ksmp.base.rte \
		   container.sys.cabinet1.add%devices.rs6ksmp.base.rte \
		   container.sys.smpdrawer1.add%devices.rs6ksmp.base.rte \
		   container.cabinet.sif1.add%devices.rs6ksmp.base.rte \
		   container.cabinet.power_supply1.add%devices.rs6ksmp.base.rte \
		   container.cabinet.op_panel1.add%devices.rs6ksmp.base.rte

ROOT_ODMADD_LIST  += cfgrule.sys_p.add%devices.rs6ksmp.base.rte

INFO_FILES        += devices.rs6ksmp.base.rte.prereq
