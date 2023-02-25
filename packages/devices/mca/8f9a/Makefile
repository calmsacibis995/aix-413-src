# @(#)32        1.3  src/packages/devices/mca/8f9a/Makefile, pkgneptune, pkg411, GOLD410 6/24/94 11:51:43
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
VPATH		 = rte:ucode:X11:diag
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/neptune/rms/gairms

BFF		= devices.mca.8f9a

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8f9a.rte \
		  devices.mca.8f9a.ucode \
		  devices.mca.8f9a.X11 \
		  devices.mca.8f9a.diag

.include <rte/packdep.mk>
.include <ucode/packdep.mk>
.include <X11/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
