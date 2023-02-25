# @(#)34        1.5  src/packages/bos/adt/prof/packdep.mk, pkgadtprof, pkg41J, 9516A_all 4/12/95 13:59:49
#
# COMPONENT_NAME: pkgadtprof
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

USR_LIBLPP_LIST		+= bos.adt.prof.namelist bos.adt.prof.rm_inv

USR_ODMADD_LIST		+=

INFO_FILES		+= bos.adt.prof.supersede bos.adt.prof.inv_u
