# @(#)45        1.6  src/packages/bos/perf/diag_tool/packdep.mk, pkgperf, pkg411, 9438C411a 9/22/94 14:37:05
#
# COMPONENT_NAME: pkgperf
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

USR_LIBLPP_LIST         += bos.perf.diag_tool.pre_rm

ROOT_LIBLPP_LIST        += bos.perf.diag_tool.unconfig \
			   bos.perf.diag_tool.cfgfiles

INFO_FILES		+= bos.perf.diag_tool.insize \
			   bos.perf.diag_tool.prereq
