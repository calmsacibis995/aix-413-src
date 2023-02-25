# @(#)34        1.4  src/packages/devices/mca/6e6c/Makefile, pkgdevgraphics, pkg411, GOLD410 2/24/94 10:58:00
#
# COMPONENT_NAME: pkgdevgraphics
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
VPATH		= rte:diag:ucode
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.6e6c

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.6e6c.rte \
		  devices.mca.6e6c.diag

#		  devices.mca.6e6c.ucode \

.include <rte/packdep.mk>
.include <diag/packdep.mk>
#.include <ucode/packdep.mk>

.include <${RULES_MK}>
