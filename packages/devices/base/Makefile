# @(#)64        1.8  src/packages/devices/base/Makefile, pkgdevbase, pkg411, GOLD410 4/1/94 14:51:45
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

VPATH		 = com:rte
VPATH	       := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}:${MAKETOP}/bos/objclass


BFF		= devices.base

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.base.rte \
		  devices.base.com

ROOT_OPTIONS    = devices.base.rte \
		  devices.base.com



.include <com/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
