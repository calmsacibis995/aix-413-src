# @(#)66        1.3  src/packages/devices/pci/0e100091/Makefile, pkgrspc, pkg41J, 9515A_all 4/6/95 11:08:27
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
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = rte:X11:diag
.if exists (${MAKETOP}/gos/rms/gairms)
VPATH		:= ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		 = devices.pci.0e100091

ILIST		 = ${BFF}
IDIR		 = /

OPTIONS		= devices.pci.0e100091.rte  \
			devices.pci.0e100091.diag
.if exists (${MAKETOP}/gos/rms/gairms)
OPTIONS		+= devices.pci.0e100091.X11
.endif

.include <rte/packdep.mk>
.if exists (${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.endif
.include <diag/packdep.mk>

.include <${RULES_MK}>
