# @(#)14        1.7  src/packages/devices/buc/00004005/Makefile, pkgdevgraphics, pkg411, GOLD410 3/7/94 17:41:59
#
# COMPONENT_NAME: pkgdevgraphics
#
# FUNCTIONS: Makefile
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
VPATH		= rte:diag:X11
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
.if exists(${MAKETOP}/gos/rms/gairms)
VPATH           := ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif

BFF		= devices.buc.00004005

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.buc.00004005.rte \
		  devices.buc.00004005.diag
.if exists(${MAKETOP}/gos/rms/gairms)
OPTIONS		+= devices.buc.00004005.X11
.endif

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.if exists(${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.endif

.include <${RULES_MK}>
