# @(#)19        1.6  src/packages/devices/mca/8f9a/rte/packdep.mk, pkgneptune, pkg411, 9436B411a 9/6/94 20:23:45
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
# (C) COPYRIGHT International Business Machines Corp. 1994
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

USR_LIBLPP_LIST		+= devices.mca.8f9a.rte.post_i \
			   devices.mca.8f9a.rte.ccm.odmdel \
			   devices.mca.8f9a.rte.pre_d \
			   devices.mca.8f9a.rte.cfginfo \
			   devices.mca.8f9a.rte.namelist \
			   devices.mca.8f9a.rte.rm_inv

INFO_FILES		+= devices.mca.8f9a.rte.prereq \
			   devices.mca.8f9a.rte.supersede
