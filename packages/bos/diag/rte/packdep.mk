# @(#)66	1.17  src/packages/bos/diag/rte/packdep.mk, pkgdiag, pkg41J, 9513A_all 3/27/95 14:41:32
#
# COMPONENT_NAME: PKGDIAG
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
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

USR_LIBLPP_LIST		+= bos.diag.rte.rm_inv \
			   bos.diag.rte.namelist

ROOT_LIBLPP_LIST	+= bos.diag.rte.config \
			   bos.diag.rte.unconfig \
			   bos.diag.rte.err	\
			   bos.diag.rte.trc

ROOT_LIBLPP_UPDT_LIST	+= bos.diag.rte.pre_u \
			   bos.diag.rte.post_u \
			   bos.diag.rte.unpre_u \
			   bos.diag.rte.unpost_u


USR_ODMADD_LIST		+= DSMOpt.add%bos.diag.rte \
			   DSMenu.add%bos.diag.rte \
			   diag.base.add%bos.diag.rte  \
			   sm_diag.add%bos.diag.rte \
			   drawer.add%bos.diag.rte \
			   diag.isasrvaid.add%bos.diag.rte

INFO_FILES		+= 
