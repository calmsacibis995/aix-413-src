# @(#)52        1.7  src/packages/devices/pci/3353b088/Makefile, pkgrspc, pkg41J, 9512A_all 3/20/95 11:53:10   
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
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = rte:X11
.if exists (${MAKETOP}/gos/rms/gairms)
VPATH		:= ${VPATH}:${MAKETOP}/gos/rms/gairms
.endif
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass/${TARGET_MACHINE}

BFF		= devices.pci.3353b088

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.pci.3353b088.rte
.if exists (${MAKETOP}/gos/rms/gairms)
OPTIONS		+= devices.pci.3353b088.X11
.endif

.include <rte/packdep.mk>
.if exists (${MAKETOP}/gos/rms/gairms)
.include <X11/packdep.mk>
.endif

.include <${RULES_MK}>
