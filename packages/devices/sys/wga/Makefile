# @(#)71        1.6  src/packages/devices/sys/wga/Makefile, pkgdevgraphics, pkg411, GOLD410 2/23/94 16:23:25
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

VPATH		 = rte:diag:X11
.if exists (${MAKETOP}/gos/rms/gairms)
VPATH		:= ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.sys.wga

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.sys.wga.diag \
 		  devices.sys.wga.rte
.if exists (${MAKETOP}/gos/rms/gairms)
OPTIONS		+= devices.sys.wga.X11
.endif

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.if exists (${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.endif

.include <${RULES_MK}>
