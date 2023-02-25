# @(#)50        1.7  src/packages/devices/mca/deff/Makefile, pkgdevcommo, pkg411, 9439B411a 9/28/94 17:44:15
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
VPATH		 = rte:rte/usr:rte/root:diag:diag/usr:sdlc:sdlc/root
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.deff

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.deff.rte \
		  devices.mca.deff.sdlc \
		  devices.mca.deff.diag

ROOT_OPTIONS	= devices.mca.deff.rte \
		  devices.mca.deff.sdlc

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <sdlc/packdep.mk>

.include <${RULES_MK}>
