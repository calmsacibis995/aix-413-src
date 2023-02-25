# @(#)57        1.2  src/packages/bos/rte/up/packdep.mk, pkgbos, pkg411, GOLD410 2/23/94 10:00:10
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

USR_LIBLPP_LIST		+= bos.rte.up.config bos.rte.up.unconfig \
			   bos.rte.up.pre_d

ROOT_LIBLPP_LIST	+= bos.rte.up.config bos.rte.up.unconfig
