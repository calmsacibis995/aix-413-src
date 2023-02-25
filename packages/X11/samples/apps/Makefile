# @(#)71        1.3  src/packages/X11/samples/apps/Makefile, pkgx, pkg411, GOLD410 3/17/94 14:36:11
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

VPATH		:=	aixclients
VPATH		:=	${VPATH}:clients
VPATH		:=	${VPATH}:demos
VPATH		:=	${VPATH}:motifdemos

BFF			 =	X11.samples

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.samples.apps.aixclients		\
				X11.samples.apps.clients		\
				X11.samples.apps.demos			\
				X11.samples.apps.motifdemos

.include <aixclients/packdep.mk>
.include <clients/packdep.mk>
.include <demos/packdep.mk>
.include <motifdemos/packdep.mk>

.include <${RULES_MK}>
