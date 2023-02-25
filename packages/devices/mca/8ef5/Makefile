# @(#)26        1.10  src/packages/devices/mca/8ef5/Makefile, pkgdevcommo, pkg411, GOLD410 7/5/94 16:15:05
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
VPATH            = rte:diag:com
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ent
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ent/root
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/entdiag
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/entdiag/root

BFF             = devices.mca.8ef5

ILIST           = ${BFF}
IDIR            = /

OPTIONS         = devices.mca.8ef5.rte \
                  devices.mca.8ef5.com \
                  devices.mca.8ef5.diag

ROOT_OPTIONS    = devices.mca.8ef5.rte \
                  devices.mca.8ef5.diag

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <com/packdep.mk>

.include <${RULES_MK}>
