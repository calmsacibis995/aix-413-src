# @(#)55        1.14  src/packages/bos/sysmgt/nim/master/packdep.mk, pkgnim, pkg41J, 9511A_all 3/7/95 17:33:20
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

USR_LIBLPP_LIST		+= bos.sysmgt.nim.master.pre_i
USR_LIBLPP_LIST		+= bos.sysmgt.nim.master.unpre_i
USR_LIBLPP_LIST		+= bos.sysmgt.nim.master.config
USR_LIBLPP_LIST		+= bos.sysmgt.nim.master.unconfig
USR_LIBLPP_LIST		+= bos.sysmgt.nim.master.pre_d
USR_LIBLPP_LIST		+= bos.sysmgt.nim.master.cfgfiles

USR_ODMADD_LIST		+= sm_nim_master.add%bos.sysmgt.nim.master

INFO_FILES		+= bos.sysmgt.nim.master.prereq
INFO_FILES		+= bos.sysmgt.nim.master.inv_u

PRELOADED_ROOT_ODMADD_LIST      +=cpp.pd.add%bos.sysmgt.nim.master

