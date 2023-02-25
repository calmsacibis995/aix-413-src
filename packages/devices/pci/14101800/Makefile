# @(#)13	1.3  src/packages/devices/pci/14101800/Makefile, pkgrspc, pkg41J, 9514A_all 4/4/95 14:48:22
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

VPATH		 = diag:rte
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass
VPATH           := ${VPATH}:${MAKETOP}/rspc/kernext/pci/stok
VPATH           := ${VPATH}:${MAKETOP}/rspc/kernext/pci/stok/root

BFF		= devices.pci.14101800

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= 	devices.pci.14101800.rte \
			devices.pci.14101800.diag

ROOT_OPTIONS    = 	devices.pci.14101800.rte

.include <diag/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
