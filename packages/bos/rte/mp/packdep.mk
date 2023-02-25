# @(#)66        1.3  src/packages/bos/rte/mp/packdep.mk, pkgbos, pkg412, 9446B 11/15/94 15:22:54
#
# COMPONENT_NAME: pkgbos
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts or prereq files would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= bos.rte.mp.config bos.rte.mp.unconfig \
			   bos.rte.mp.pre_d

ROOT_LIBLPP_LIST	+= bos.rte.mp.config bos.rte.mp.unconfig

USR_ODMADD_LIST		+= sm_rds.add%bos.rte.mp
