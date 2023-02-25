# @(#)13        1.4  src/packages/X11/adt/Makefile, pkgx, pkg411, GOLD410 3/15/94 14:58:42
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

VPATH		 =	bitmaps
VPATH		:=	${VPATH}:ext
VPATH		:=	${VPATH}:imake
VPATH		:=	${VPATH}:include
VPATH		:=	${VPATH}:lib
VPATH		:=	${VPATH}:motif

BFF			 =	X11.adt

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.adt.bitmaps		\
				X11.adt.ext			\
				X11.adt.imake		\
				X11.adt.include		\
				X11.adt.lib			\
				X11.adt.motif

.include <bitmaps/packdep.mk>
.include <ext/packdep.mk>
.include <imake/packdep.mk>
.include <include/packdep.mk>
.include <lib/packdep.mk>
.include <motif/packdep.mk>


.include <${RULES_MK}>
