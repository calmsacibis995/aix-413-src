# @(#)35        1.4  src/packages/devices/sio/sa/Makefile, pkgdevtty, pkg411, GOLD410 2/25/94 09:37:18
#
# COMPONENT_NAME: pkgdevtty
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
VPATH		 = diag:rte
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.sio.sa

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.sio.sa.rte \
		  devices.sio.sa.diag

.include <diag/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
