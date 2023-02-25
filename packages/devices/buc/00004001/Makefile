# @(#)72        1.11  src/packages/devices/buc/00004001/Makefile, pkgdevgraphics, pkg411, GOLD410 7/6/94 11:45:42
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

VPATH		 = rte:diag:X11:com:X11/com
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
.if exists(${MAKETOP}/gos/rms/gairms)
VPATH           := ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif

BFF		= devices.buc.00004001

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.buc.00004001.rte \
		  devices.buc.00004001.diag
.if exists(${MAKETOP}/gos/rms/gairms)
OPTIONS		+= \
		  devices.buc.00004001.X11.com \
		  devices.buc.00004001.X11
.endif

ROOT_OPTIONS	= devices.buc.00004001.com

.include <diag/packdep.mk>
.include <rte/packdep.mk>
.if exists(${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.include <X11/com/packdep.mk>
.endif

.include <${RULES_MK}>
