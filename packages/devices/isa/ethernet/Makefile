# @(#)03        1.2  src/packages/devices/isa/ethernet/Makefile, pkgrspc, pkg411, GOLD410 7/5/94 14:19:39
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

VPATH            = rte:rte/root
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass
VPATH           := ${VPATH}:${MAKETOP}/rspc/kernext/isa/ient

BFF             = devices.isa.ethernet

ILIST		= ${BFF}
IDIR		= /

OPTIONS         = devices.isa.ethernet.rte
ROOT_OPTIONS    = devices.isa.ethernet.rte

.include <rte/packdep.mk>

.include <${RULES_MK}>
