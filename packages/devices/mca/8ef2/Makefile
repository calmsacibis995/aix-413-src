# @(#)42        1.10  src/packages/devices/mca/8ef2/Makefile, pkgdevcommo, pkg411, GOLD410 7/5/94 15:01:03
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

VPATH		= com:rte:diag:diag/com
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ient
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ient/root
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ientdiag
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ientdiag/root

BFF		= devices.mca.8ef2

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8ef2.com \
		  devices.mca.8ef2.rte \
		  devices.mca.8ef2.diag \
		  devices.mca.8ef2.diag.com

ROOT_OPTIONS	= devices.mca.8ef2.com \
		  devices.mca.8ef2.diag.com

.include <com/packdep.mk>
.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <diag/com/packdep.mk>

.include <${RULES_MK}>
