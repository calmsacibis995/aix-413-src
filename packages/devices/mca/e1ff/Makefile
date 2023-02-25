# @(#)35        1.4  src/packages/devices/mca/e1ff/Makefile, pkgdevcommo, pkg411, GOLD410 2/10/94 17:49:07
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

VPATH		= rte:diag
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.e1ff

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.e1ff.rte \
		  devices.mca.e1ff.diag 

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
