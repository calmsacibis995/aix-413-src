# @(#)90        1.7  src/packages/X11/base/Makefile, pkgx, pkg411, GOLD410 6/13/94 09:39:20
#
# COMPONENT_NAME: pkgx
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 =	rte
VPATH		:=	${VPATH}:rte/root
VPATH		:=	${VPATH}:smt
VPATH		:=	${VPATH}:smt/root
VPATH		:=	${VPATH}:common
VPATH		:=	${VPATH}:lib
VPATH		:=	${VPATH}:${MAKETOP}/gos/2d/XTOP/xinput/server/ddx

BFF		 =	X11.base

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.base.rte	\
			X11.base.smt	\
			X11.base.common	\
			X11.base.lib

ROOT_OPTIONS =	X11.base.rte	\
			X11.base.smt

.include <rte/packdep.mk>
.include <smt/packdep.mk>
.include <common/packdep.mk>
.include <lib/packdep.mk>

.include <${RULES_MK}>
