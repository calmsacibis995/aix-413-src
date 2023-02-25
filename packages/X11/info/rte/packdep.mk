# @(#)71	1.3  src/packages/X11/info/rte/packdep.mk, pkginfo, pkg411, GOLD410 4/13/94 15:30:56
#
# COMPONENT_NAME: pkginfo
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
#		config scripts or prereq files would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= X11.info.rte.namelist \
			   X11.info.rte.cfgfiles \
			   X11.info.rte.rm_inv

ROOT_LIBLPP_LIST	+= X11.info.rte.config \
			   X11.info.rte.unconfig

INFO_FILES		+= X11.info.rte.prereq \
			   X11.info.rte.insize
