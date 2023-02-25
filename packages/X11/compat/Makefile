# @(#)79        1.9  src/packages/X11/compat/Makefile, pkgx, pkg411, GOLD410 4/14/94 17:15:53
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
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 =	samples/util
VPATH		:=	${VPATH}:fnt/pc
VPATH		:=	${VPATH}:fnt/oldX
VPATH		:=	${VPATH}:lib/X11R3
VPATH		:=	${VPATH}:lib/X11R4
VPATH		:=	${VPATH}:lib/Motif10
VPATH		:=	${VPATH}:lib/Motif114

BFF			 =	X11.compat

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.compat.samples.util			\
				X11.compat.fnt.pc				\
				X11.compat.fnt.oldX				\
				X11.compat.lib.X11R3			\
				X11.compat.lib.X11R4			\
				X11.compat.lib.Motif10			\
				X11.compat.lib.Motif114

.include <samples/util/packdep.mk>
.include <fnt/pc/packdep.mk>
.include <fnt/oldX/packdep.mk>
.include <lib/X11R3/packdep.mk>
.include <lib/X11R4/packdep.mk>
.include <lib/Motif10/packdep.mk>
.include <lib/Motif114/packdep.mk>

.include <${RULES_MK}>
