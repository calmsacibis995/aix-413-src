# @(#)02        1.8 8/12/94 src/packages/devices/buc/00004005/rte/packdep.mk, pkgdevgraphics, pkg411, 9433B411a 14:36:31
#
# COMPONENT_NAME: pkgdevgraphics
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

USR_LIBLPP_LIST		+= devices.buc.00004005.rte.post_i \
			   devices.buc.00004005.rte.ccm.odmdel \
			   devices.buc.00004005.rte.pre_d \
			   devices.buc.00004005.rte.cfginfo

INFO_FILES		+= devices.buc.00004005.rte.prereq
