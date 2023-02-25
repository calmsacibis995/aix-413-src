# @(#)98        1.7  src/packages/devices/sys/slc/Makefile, pkgdevcommo, pkg411, GOLD410 7/6/94 08:01:55
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
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/sol
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/sol/root

BFF		= devices.sys.slc

ILIST		= ${BFF}
IDIR		= /

OPTIONS 	= devices.sys.slc.rte \
		  devices.sys.slc.diag

ROOT_OPTIONS    = devices.sys.slc.rte

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
