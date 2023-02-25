# @(#)19	1.1  src/packages/blkmux/packdep.mk, pkgblkmux, pkg411, 9439C411a 9/29/94 16:31:49
#
# COMPONENT_NAME: pkgblkmux
#
# FUNCTIONS: packaging definition
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
INFO_FILES		+= blkmux.supersede \
			   blkmux.prereq

USR_LIBLPP_LIST		+= blkmux.namelist \
			   blkmux.rm_inv

USR_ODMADD_LIST		+= sm_cat.add%blkmux
