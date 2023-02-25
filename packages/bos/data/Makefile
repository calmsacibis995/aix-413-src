# @(#)76        1.6  src/packages/bos/data/Makefile, pkgbos, pkg411, GOLD410 5/20/94 11:32:03
#
# COMPONENT_NAME: pkgbos
#
# FUNCTIONS: Makefile
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

BFF		= bos.data

ILIST		= ${BFF}
IDIR		= /

SHARE_OPTIONS	= bos.data

.include <packdep.mk>

.include <${RULES_MK}>

bos.data : bos.data.data
	${CP} bos.data.data bos.data
