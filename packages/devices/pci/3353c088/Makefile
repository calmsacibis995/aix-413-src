# @(#)74	1.4  src/packages/devices/pci/3353c088/Makefile, pkgrspc, pkg41J, 9520B_all 5/17/95 09:28:36
#
# COMPONENT_NAME: pkgrspc
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = rte:X11:com:X11/com
.if exists (${MAKETOP}/gos/rms/gairms)
VPATH		:= ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.pci.3353c088

ILIST		= ${BFF}
IDIR		= /

OPTIONS		=	devices.pci.3353c088.rte \
				devices.pci.3353c088.com
.if exists (${MAKETOP}/gos/rms/gairms)
OPTIONS		+=	devices.pci.3353c088.X11 \
				devices.pci.3353c088.X11.com
.endif

.include <rte/packdep.mk>
.include <com/packdep.mk>
.if exists (${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.include <X11/com/packdep.mk>
.endif

.include <${RULES_MK}>
