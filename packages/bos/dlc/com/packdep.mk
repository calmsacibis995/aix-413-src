# @(#)07        1.7  src/packages/bos/dlc/com/packdep.mk, pkgdlc, pkg41J, 9517B_all 4/21/95 13:08:03
#
# COMPONENT_NAME: pkgdlc
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
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

ROOT_LIBLPP_LIST	+= bos.dlc.com.post_i \
                	   bos.dlc.com.unpost_i

ROOT_LIBLPP_UPDT_LIST   += bos.dlc.com.post_u \
                	   bos.dlc.com.unpost_u

INFO_FILES              += bos.dlc.com.insize

