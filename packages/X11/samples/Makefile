# @(#)57        1.5  src/packages/X11/samples/Makefile, pkgx, pkg411, GOLD410 3/22/94 09:30:03
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

VPATH		 =	lib/Core
VPATH		:=	${VPATH}:common
VPATH		:=	${VPATH}:apps/aixclients
VPATH		:=	${VPATH}:apps/clients
VPATH		:=	${VPATH}:apps/demos
VPATH		:=	${VPATH}:apps/motifdemos
VPATH		:=	${VPATH}:doc
VPATH		:=	${VPATH}:ext
VPATH		:=	${VPATH}:fnt/util
VPATH		:=	${VPATH}:im
VPATH		:=	${VPATH}:rgb

BFF			 =	X11.samples

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.samples.lib.Core			\
				X11.samples.common				\
				X11.samples.apps.aixclients		\
				X11.samples.apps.clients		\
				X11.samples.apps.demos			\
				X11.samples.apps.motifdemos		\
				X11.samples.doc					\
				X11.samples.ext					\
				X11.samples.fnt.util			\
				X11.samples.im					\
				X11.samples.rgb

.include <lib/Core/packdep.mk>
.include <common/packdep.mk>
.include <apps/aixclients/packdep.mk>
.include <apps/clients/packdep.mk>
.include <apps/demos/packdep.mk>
.include <apps/motifdemos/packdep.mk>
.include <doc/packdep.mk>
.include <ext/packdep.mk>
.include <fnt/util/packdep.mk>
.include <im/packdep.mk>
.include <rgb/packdep.mk>

.include <${RULES_MK}>
