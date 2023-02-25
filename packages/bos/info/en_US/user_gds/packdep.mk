# @(#)98        1.2  src/packages/bos/info/en_US/user_gds/packdep.mk, pkginfodb, pkg411, GOLD410 1/11/94 12:09:22
#
# COMPONENT_NAME: pkginfodb
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
USR_LIBLPP_LIST		+=

INFO_FILES		+= bos.info.en_US.user_gds.prereq
