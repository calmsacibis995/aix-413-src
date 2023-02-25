# @(#)06        1.6  src/packages/bos/terminfo/fortune/data/packdep.mk, pkgterm, pkg41J, 9515A_all 4/4/95 16:00:03
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		post_i scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

SHARE_LIBLPP_LIST		+= bos.terminfo.fortune.data.post_i

SHARE_INFO_FILES		+= bos.terminfo.fortune.data.prereq \
                		   bos.terminfo.fortune.data.insize
SHARE_LIBLPP_UPDT_LIST		+= bos.terminfo.fortune.data.post_u
