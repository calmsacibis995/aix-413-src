# @(#)92        1.6  src/packages/devices/mca/edd5/Makefile, pkgdevgraphics, pkg411, GOLD410 7/5/94 17:25:42
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
VPATH		= com:rte:diag:X11
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
.if exists(${MAKETOP}/gos/2d/XTOP/xinput/server/ddx)
VPATH		:= ${VPATH}:${MAKETOP}/gos/2d/XTOP/xinput/server/ddx
.endif

BFF		= devices.mca.edd5

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.edd5.com \
		  devices.mca.edd5.rte \
		  devices.mca.edd5.diag
.if exists(${MAKETOP}/gos/2d/XTOP/xinput/server/ddx)
OPTIONS		+= devices.mca.edd5.X11
.endif

# When adding trace back in remove the comment from the next line.
#ROOT_OPTIONS	= devices.mca.edd5.com

.include <com/packdep.mk>
.include <rte/packdep.mk>
.include <diag/packdep.mk>
.if exists(${MAKETOP}/gos/2d/XTOP/xinput/server/ddx)
.include <X11/packdep.mk>
.endif

.include <${RULES_MK}>
