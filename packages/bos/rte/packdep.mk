# @(#)85        1.4  src/packages/bos/rte/packdep.mk, pkgbos, pkg411, GOLD410 3/15/94 14:55:03
#
# COMPONENT_NAME: pkgbos
#
# FUNCTIONS: packaging definition
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

USR_LIBLPP_LIST		+= bos.rte.cfgfiles \
			   bos.rte.post_i \
			   bos.rte.pre_i \
			   bos.rte.usr.rmlist \
			   bos.rte.root.rmlist \
			   incompat.pkgs
