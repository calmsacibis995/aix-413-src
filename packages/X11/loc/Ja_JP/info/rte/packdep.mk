# @(#)94	1.1  src/packages/X11/loc/ja_JP/info/rte/packdep.mk, pkginfo, pkg411, GOLD410  4/18/94  17:24:55
#
# COMPONENT_NAME: pkginfo
#
# FUNCTIONS:
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

USR_LIBLPP_LIST		+= X11.loc.ja_JP.info.rte.namelist \
			   X11.loc.ja_JP.info.rte.cfgfiles \
			   X11.loc.ja_JP.info.rte.rm_inv

INFO_FILES		+= X11.loc.ja_JP.info.rte.prereq
