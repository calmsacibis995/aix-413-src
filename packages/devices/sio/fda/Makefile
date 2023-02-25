# @(#)00	1.7  src/packages/devices/sio/fda/Makefile, pkgdevbase, pkg411, GOLD410 6/28/94 07:48:40
#
# COMPONENT_NAME: pkgdevbase
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
VPATH		 = diag:rte:com
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.sio.fda

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.sio.fda.rte \
		  devices.sio.fda.com \
		  devices.sio.fda.diag


.include <diag/packdep.mk>
.include <rte/packdep.mk>
.include <com/packdep.mk>

.include <${RULES_MK}>
