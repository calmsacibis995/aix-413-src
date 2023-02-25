# @(#)48        1.7  src/packages/bos/sysmgt/nim/client/packdep.mk, pkgnim, pkg41J, 9524E 6/15/95 10:16:12
#
# COMPONENT_NAME: pkgnim
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

USR_LIBLPP_LIST		+= bos.sysmgt.nim.client.unconfig \
			   bos.sysmgt.nim.client.post_i
ROOT_LIBLPP_LIST		+= bos.sysmgt.nim.client.cfgfiles

USR_ODMADD_LIST		+= sm_nim_client.add%bos.sysmgt.nim.client

INFO_FILES		+= bos.sysmgt.nim.client.prereq
