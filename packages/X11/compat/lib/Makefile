# @(#)06        1.4  src/packages/X11/compat/lib/Makefile, pkgx, pkg411, GOLD410 4/14/94 17:14:45
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

VPATH		 =	X11R3
VPATH		:=	${VPATH}:X11R4
VPATH		:=	${VPATH}:Motif10
VPATH		:=	${VPATH}:Motif114

BFF			 =	X11.compat

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.compat.lib.X11R3		\
				X11.compat.lib.X11R4		\
				X11.compat.lib.Motif10		\
				X11.compat.lib.Motif114

.include <X11R3/packdep.mk>
.include <X11R4/packdep.mk>
.include <Motif10/packdep.mk>
.include <Motif114/packdep.mk>

.include <${RULES_MK}>
