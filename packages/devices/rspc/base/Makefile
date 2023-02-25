# @(#)86        1.7  src/packages/devices/rspc/base/Makefile, pkgrspc, pkg41J, 9522A_b 5/31/95 11:12:22
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
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		 = rte:com:diag:rte/root
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.rspc.base

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.rspc.base.rte \
		  devices.rspc.base.com \
		  devices.rspc.base.diag

ROOT_OPTIONS	= devices.rspc.base.rte

.include <rte/packdep.mk>
.include <com/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
