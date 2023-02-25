# @(#)35        1.10  src/packages/devices/mca/8ef4/Makefile, pkgdevcommo, pkg411, GOLD410 7/5/94 15:45:59
#
# COMPONENT_NAME: pkgdevcommo
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

VPATH           = rte:diag:ucode:com
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/fddi
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/fddi/root
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/fddidiag
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/fddidiag/root

BFF             = devices.mca.8ef4

ILIST           = ${BFF}
IDIR            = /

OPTIONS         = devices.mca.8ef4.rte \
                  devices.mca.8ef4.com \
                  devices.mca.8ef4.ucode \
                  devices.mca.8ef4.diag

ROOT_OPTIONS    = devices.mca.8ef4.rte \
                  devices.mca.8ef4.diag

.include <rte/packdep.mk>
.include <com/packdep.mk>
.include <diag/packdep.mk>
.include <ucode/packdep.mk>

.include <${RULES_MK}>
