# @(#)06	1.3  src/packages/bos/ifor_ls/server/packdep.mk, pkgnetls, pkg411, GOLD410 7/13/94 10:09:16
#
# COMPONENT_NAME: pkgnetls
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

USR_LIBLPP_LIST		+= bos.ifor_ls.server.namelist \
			   bos.ifor_ls.server.rm_inv

ROOT_LIBLPP_LIST	+= bos.ifor_ls.server.config \
                           bos.ifor_ls.server.unconfig \
			   bos.ifor_ls.server.namelist \
			   bos.ifor_ls.server.rm_inv

USR_ODMADD_LIST		+= netls2.add%bos.ifor_ls.server

INFO_FILES		+= bos.ifor_ls.server.prereq
