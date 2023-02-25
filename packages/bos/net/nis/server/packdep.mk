# @(#)76        1.6  src/packages/bos/net/nis/server/packdep.mk, pkgnis, pkg411, GOLD410 4/18/94 10:04:00
#
# COMPONENT_NAME: pkgnis
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
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= bos.net.nis.server.namelist \
                           bos.net.nis.server.rm_inv

ROOT_LIBLPP_LIST	+= bos.net.nis.server.config \
			   bos.net.nis.server.unconfig \
			   bos.net.nis.server.pre_i \
                           bos.net.nis.server.namelist \
                           bos.net.nis.server.rm_inv

INFO_FILES		+= bos.net.nis.server.prereq

USR_ODMADD_LIST         += sm_nis_server.add%bos.net.nis.server
