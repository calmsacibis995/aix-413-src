# @(#)49	1.4  src/packages/bos/compat/net/packdep.mk, pkgnetcmpt, pkg411, GOLD410 6/10/94 09:12:53
#
# COMPONENT_NAME: pkgcompat
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

ROOT_LIBLPP_LIST	+= bos.compat.net.config

USR_LIBLPP_LIST		+= bos.compat.net.namelist \
			   bos.compat.net.rm_inv

INFO_FILES              += bos.compat.net.prereq
