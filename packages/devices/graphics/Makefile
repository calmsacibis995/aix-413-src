# @(#)65        1.8  src/packages/devices/graphics/Makefile, pkgdevgraphics, pkg41J, 9514A_all 3/31/95 14:06:14
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
VPATH		= com:com/root:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/kernext/lft:${MAKETOP}/bos/kernext/lft/root
VPATH           := ${VPATH}:voo

BFF		= devices.graphics

ILIST		= ${BFF}
IDIR		= /

OPTIONS         = devices.graphics.com     \
                  devices.graphics.voo

ROOT_OPTIONS    = devices.graphics.com

.include <com/packdep.mk>
.include <voo/packdep.mk>

.include <${RULES_MK}>
