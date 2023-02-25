# @(#)50        1.3  src/packages/bos/data/packdep.mk, pkgbos, pkg411, GOLD410 2/1/94 15:32:08
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

SHARE_LIBLPP_LIST	+= bos.data.namelist bos.data.rm_inv
