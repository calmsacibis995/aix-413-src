# @(#)28        1.5  src/packages/bos/acct/packdep.mk, pkgacct, pkg411, GOLD410 5/27/94 15:21:41
#
# COMPONENT_NAME: pkgacct
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts or prereq files would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#
INFO_FILES		+= bos.acct.insize  \
			   bos.acct.supersede

USR_LIBLPP_LIST		+= bos.acct.namelist \
			   bos.acct.rm_inv

ROOT_LIBLPP_LIST	+= bos.acct.namelist \
			   bos.acct.cfgfiles \
			   bos.acct.rm_inv

USR_ODMADD_LIST     += sm_bosacct.add%bos.acct
