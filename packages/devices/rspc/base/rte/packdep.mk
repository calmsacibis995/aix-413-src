# @(#)44        1.10  src/packages/devices/rspc/base/rte/packdep.mk, pkgrspc, pkg41J, 9525E_all 6/21/95 09:58:00
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

USR_LIBLPP_LIST		+= devices.rspc.base.rte.pre_d 

USR_ODMADD_LIST		+= planar.sys.sysplanar_rspc.add%devices.rspc.base.rte \
			   sys.node.rspc.add%devices.rspc.base.rte  \
			   processor.sys.proc_rspc.add%devices.rspc.base.rte  \
			   memory.sys.totmem.add%devices.rspc.base.rte  \
			   memory.sys.L2cache_rspc.add%devices.rspc.base.rte  


ROOT_LIBLPP_LIST	+= devices.rspc.base.rte.post_i

ROOT_LIBLPP_UPDT_LIST   += devices.rspc.base.rte.post_u \
                           devices.rspc.base.rte.unpost_u 

ROOT_ODMADD_LIST	+= 
INFO_FILES		+= devices.rspc.base.rte.prereq


