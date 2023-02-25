# @(#)84        1.7  src/packages/bos/sysmgt/trace/packdep.mk, pkgtrc, pkg411, GOLD410 6/24/94 18:24:21
#
# COMPONENT_NAME: pkgtrc
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

USR_LIBLPP_LIST		+= bos.sysmgt.trace.namelist
ROOT_LIBLPP_LIST	+= bos.sysmgt.trace.post_i \
				bos.sysmgt.trace.namelist \
				bos.sysmgt.trace.unpost_i \
				bos.sysmgt.trace.trc

USR_ODMADD_LIST		+= trace.add%bos.sysmgt.trace
ROOT_ODMADD_LIST	+=
