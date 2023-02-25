# @(#)91        1.11  src/packages/bos/sysmgt/smit/packdep.mk, pkgsmit, pkg41J, 9523B_all 5/26/95 18:43:13
#
# COMPONENT_NAME: pkgsmit
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

USR_LIBLPP_LIST		+= bos.sysmgt.smit.namelist \
			   bos.sysmgt.smit.post_i \
			   bos.sysmgt.smit.unpost_i

ROOT_LIBLPP_LIST	+= 

USR_ODMADD_LIST		+= inst_menus.add%bos.sysmgt.smit \
			   instupdt.add%bos.sysmgt.smit
ROOT_ODMADD_LIST	+=
