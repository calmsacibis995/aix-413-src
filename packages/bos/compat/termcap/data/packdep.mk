# @(#)85	1.3  src/packages/bos/compat/termcap/data/packdep.mk, pkgtermcap, pkg41J 6/2/95 09:17:26
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
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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

SHARE_LIBLPP_LIST               += bos.compat.termcap.data.namelist \
				   bos.compat.termcap.data.rm_inv

SHARE_ODMADD_LIST		+=

SHARE_INFO_FILES		+= bos.compat.termcap.data.inv_u
