# @(#)60        1.11 9/20/94 src/packages/devices/graphics/com/packdep.mk, pkgdevgraphics, pkg411, 9438B411a 17:41:09
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

USR_LIBLPP_LIST		+= devices.graphics.com.pre_d \
			   devices.graphics.com.cfginfo

ROOT_LIBLPP_LIST	+= devices.graphics.com.trc \
			   devices.graphics.com.unconfig

USR_ODMADD_LIST		+= lft.add%devices.graphics.com		\
			   lft_dpm.add%devices.graphics.com	\
			   sm_lft.add%devices.graphics.com 	\
			   rcm.add%devices.graphics.com

ROOT_ODMADD_LIST        += cfgrule.lft.add%devices.graphics.com
