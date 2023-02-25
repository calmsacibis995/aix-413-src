# @(#)50	1.2  src/packages/devices/mca/f6f8/Makefile, pkgdevgraphics, pkg41J, 9509A_all 2/17/95 13:46:12
#
# COMPONENT_NAME:  pkgdevgraphics
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
VPATH		= rte:diag
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.f6f8

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.f6f8.rte \
                  devices.mca.f6f8.diag

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
