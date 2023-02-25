# @(#)15        1.5  src/packages/devices/sio/ppa/Makefile, pkgdevbase, pkg411, GOLD410 5/4/94 11:28:32
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

BFF		= devices.sio.ppa

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.sio.ppa.rte \
		  devices.sio.ppa.diag \
		  devices.sio.ppa.com

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <com/packdep.mk>

.include <${RULES_MK}>
