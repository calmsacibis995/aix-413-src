# @(#)22	1.6  src/packages/bos/net/ncs/packdep.mk, pkgncs, pkg411, GOLD410 7/13/94 10:09:53
#
#
# COMPONENT_NAME: pkgncs
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
# *_LIBLPP_LIST contains the list of files that need to be added to
#               the respective liblpp.a other than the copyright, .al,
#               .tcb, .size, and .inventory files.  For example, any
#               config scripts would be included here.
#               The variable may be left blank if there are no additional
#               files, but the option will exist.
#


USR_LIBLPP_LIST		+= bos.net.ncs.namelist \
			   bos.net.ncs.rm_inv

ROOT_LIBLPP_LIST	+= bos.net.ncs.config \
			   bos.net.ncs.unconfig \
			   bos.net.ncs.pre_i \
			   bos.net.ncs.cfgfiles \
			   bos.net.ncs.namelist \
			   bos.net.ncs.rm_inv

INFO_FILES		+= bos.net.ncs.supersede
