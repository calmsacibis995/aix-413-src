# @(#)23        1.5  src/packages/devices/mca/efc6/Makefile, pkgdevgraphics, pkg411, GOLD410 4/27/94 09:39:44
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
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= X11:rte:diag
.if exists (${MAKETOP}/gos/rms/gairms)
VPATH		:= ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF			= devices.mca.efc6

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.efc6.diag \
                  devices.mca.efc6.rte
.if exists (${MAKETOP}/gos/rms/gairms)
OPTIONS		+= devices.mca.efc6.X11
.endif

ROOT_OPTIONS	= devices.mca.efc6.rte

.if exists (${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.endif
.include <diag/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
