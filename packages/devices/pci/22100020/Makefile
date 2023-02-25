# @(#)54	1.2  src/packages/devices/pci/22100020/Makefile, pkgrspc, pkg41J, 9514A_all 4/4/95 10:25:20
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

VPATH            = rte:diag
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass
VPATH           := ${VPATH}:${MAKETOP}/rspc/kernext/pci/kent
VPATH           := ${VPATH}:${MAKETOP}/rspc/kernext/pci/kent/root

BFF             = devices.pci.22100020

ILIST		= ${BFF}
IDIR		= /

OPTIONS         = devices.pci.22100020.rte \
		  devices.pci.22100020.diag

ROOT_OPTIONS    = devices.pci.22100020.rte

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
