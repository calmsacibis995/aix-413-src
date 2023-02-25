# @(#)29        1.9  src/packages/devices/mca/8fc8/Makefile, pkgdevcommo, pkg411, GOLD410 7/5/94 16:50:33
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

VPATH            = rte:diag:ucode:com
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/tok
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/tok/root
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/tokdiag
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/tokdiag/root

BFF		= devices.mca.8fc8

ILIST		= ${BFF}
IDIR		= /

OPTIONS         = devices.mca.8fc8.rte \
		  devices.mca.8fc8.com \
		  devices.mca.8fc8.diag \
		  devices.mca.8fc8.ucode

ROOT_OPTIONS    = devices.mca.8fc8.rte \
		  devices.mca.8fc8.diag

.include <rte/packdep.mk>
.include <com/packdep.mk>
.include <diag/packdep.mk>
.include <ucode/packdep.mk>

.include <${RULES_MK}>
