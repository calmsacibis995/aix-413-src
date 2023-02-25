# @(#)08        1.7  src/packages/devices/sio/ktma/Makefile, pkgdevgraphics, pkg411, GOLD410 7/6/94 08:01:06
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
VPATH		 = diag:rte:com
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.sio.ktma

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.sio.ktma.com \
		  devices.sio.ktma.rte \
		  devices.sio.ktma.diag

ROOT_OPTIONS	= devices.sio.ktma.com

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <com/packdep.mk>

.include <${RULES_MK}>
