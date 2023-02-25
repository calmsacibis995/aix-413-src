# @(#)56        1.8  src/packages/bos/powermgt/rte/packdep.mk, pkgpwrmgt, pkg41J, 9524D_all 6/13/95 15:39:15
#
# COMPONENT_NAME: pkgpwrmgt
#
# FUNCTIONS: packaging definition
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

USR_LIBLPP_LIST		+=
ROOT_LIBLPP_LIST	+= bos.powermgt.rte.config \
			   bos.powermgt.rte.unconfig
ROOT_LIBLPP_UPDT_LIST	+= bos.powermgt.rte.config_u \
			   bos.powermgt.rte.unconfig_u

USR_ODMADD_LIST		+= pwrcmd.add%bos.powermgt.rte
ROOT_ODMADD_LIST	+=

INFO_FILES		+= bos.powermgt.rte.prereq
