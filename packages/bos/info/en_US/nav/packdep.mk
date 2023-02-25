# @(#)99        1.4  src/packages/bos/info/en_US/nav/packdep.mk, pkginfodb, pkg411, 9439C411e 10/2/94 14:52:12
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
USR_LIBLPP_LIST		+= bos.info.en_US.nav.post_i

INFO_FILES		+= bos.info.en_US.nav.prereq
