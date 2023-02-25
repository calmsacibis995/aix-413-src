# @(#)38        1.9  src/packages/devices/mca/8ee3/Makefile, pkgdevgraphics, pkg411, GOLD410 3/10/94 10:29:12
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
VPATH		 = rte:diag:ucode:X11
.if exists(${MAKETOP}/gos/rms/gairms)
VPATH		:= ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.8ee3

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8ee3.diag \
		  devices.mca.8ee3.rte \
		  devices.mca.8ee3.ucode
.if exists(${MAKETOP}/gos/rms/gairms)
OPTIONS		+= devices.mca.8ee3.X11
.endif


.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <ucode/packdep.mk>
.if exists(${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.endif

.include <${RULES_MK}>
