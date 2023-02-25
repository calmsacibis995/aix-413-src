# @(#)64        1.10  src/packages/devices/buc/00004002/Makefile, pkgdevgraphics, pkg41J, 9511A_all 3/15/95 09:32:25
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
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= rte:diag:X11
VPATH		:=${VPATH}:${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
.if exists (${MAKETOP}/gos/rms/gairms)
VPATH		:=${VPATH}:${MAKETOP}/gos/rms/gairms
.endif

BFF		= devices.buc.00004002

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.buc.00004002.rte \
		 devices.buc.00004002.diag
.if exists (${MAKETOP}/gos/rms/gairms)
OPTIONS		+= devices.buc.00004002.X11
.endif


.include <rte/packdep.mk>
.include <diag/packdep.mk>
.if exists (${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.endif

.include <${RULES_MK}>
