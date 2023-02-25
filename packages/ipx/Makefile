# @(#)59        1.2  src/packages/ipx/Makefile, pkgnetware, pkg411, 9438B411a 9/19/94 09:04:55
#
# COMPONENT_NAME: pkginfodb
#
# FUNCTIONS: Makefile
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
# SUBDIRS		= rte api

VPATH+=:api:rte:api/root:rte/root

BFF	= ipx

ILIST	= $(BFF)
IDIR	= /

LPPVERSION	= 04
LPPRELEASE	= 01
LPPMAINT		= 00
LPPFIXLEVEL	= 00

OPTIONS	= ipx.api \
           ipx.rte

ROOT_OPTIONS	= ipx.rte

.include <api/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
