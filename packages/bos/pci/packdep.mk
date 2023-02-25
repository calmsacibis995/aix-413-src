# @(#)83        1.4  src/packages/bos/pci/packdep.mk, pkgpci, pkg410 4/9/94 16:0# 7:31
#
# COMPONENT_NAME: pkgpci
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
#
#
USR_LIBLPP_LIST		+= lpp.doc bos.pci.pre_i \
			   bos.pci.namelist

ROOT_LIBLPP_LIST	+= bos.pci.config bos.pci.unconfig \
			   bos.pci.namelist	

INFO_FILES		+= bos.pci.prereq \
			   bos.pci.supersede

