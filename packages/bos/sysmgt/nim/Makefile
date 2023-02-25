# @(#)18        1.4  src/packages/bos/sysmgt/nim/Makefile, pkgnim, pkg411, 9435B411a 8/30/94 17:23:28
#
# COMPONENT_NAME: pkgnim
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
VPATH		 = client:master:spot
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/nim/objrepos
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass

BFF		= bos.sysmgt

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.sysmgt.nim.client \
		  bos.sysmgt.nim.master \
		  bos.sysmgt.nim.spot

ROOT_OPTIONS	= bos.sysmgt.nim.client \
		  bos.sysmgt.nim.master

.include <client/packdep.mk>
.include <master/packdep.mk>
.include <spot/packdep.mk>

.include <${RULES_MK}>
