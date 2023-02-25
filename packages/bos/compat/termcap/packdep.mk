# @(#)83	1.1  src/packages/bos/compat/termcap/packdep.mk, pkgtermcap, pkg411, GOLD410 3/8/94 15:53:46
#
# COMPONENT_NAME: pkgtermcap
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
#		post_i scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST			+= bos.compat.termcap.namelist \
				   bos.compat.termcap.rm_inv

USR_ODMADD_LIST			+=
