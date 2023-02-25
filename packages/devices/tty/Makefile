# @(#)81        1.8  src/packages/devices/tty/Makefile, pkgdevtty, pkg411, GOLD410 3/23/94 12:24:00
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
VPATH		 = rte
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass

BFF		= devices.tty

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.tty.rte

ROOT_OPTIONS    = devices.tty.rte

.include <rte/packdep.mk>

.include <${RULES_MK}>
