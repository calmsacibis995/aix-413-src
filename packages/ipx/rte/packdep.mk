# @(#)64 1.4  src/packages/ipx/rte/packdep.mk, pkgnetware, pkg411, 9439B411a 9/27/94 14:36:19
#
# COMPONENT_NAME: netware
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


USR_LIBLPP_LIST	+=  \
	ipx.rte.namelist \
	ipx.rte.rm_inv \
	ipx.rte.pre_d \
	ipx.rte.pre_rm

ROOT_LIBLPP_LIST	+= \
	ipx.rte.config \
	ipx.rte.unconfig \
	ipx.rte.namelist \
	ipx.rte.rm_inv \
	ipx.rte.cfgfiles
