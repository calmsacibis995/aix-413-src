# @(#)60        1.1  src/packages/bos/adt/client/Makefile, pkgadt, pkg41J, 9522A_all 5/26/95 15:39:54
#
# COMPONENT_NAME: pkgadt
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PTF_UPDATE	= no

VPATH		= ../lib

BFF		= bos.adt

ILIST		= ${BFF}.client
IDIR		= /

OPTIONS		= bos.adt.lib

.include <../lib/packdep.mk>

.include <${RULES_MK}>

bos.adt.client : bos.adt
	${CP} bos.adt bos.adt.client
