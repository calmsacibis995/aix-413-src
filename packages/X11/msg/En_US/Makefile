# @(#)84        1.28  src/packages/X11/msg/en_US/Makefile, pkgx, pkg411, 9440G411a 10/13/94 11:32:39
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
# This Makefile is written to build multiple message locales from the en_US.
# The contents of this file should be looked at carefully prior to
# modifications.

MSGLANG		?=	en_US

ENG			 =	${MAKETOP}/packages/X11/msg/en_US

VPATH		 =	${ENG}/base/rte
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/base/rte
VPATH		:=	${VPATH}:${ENG}/base/common
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/base/common
VPATH		:=	${VPATH}:${ENG}/motif/lib
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/motif/lib
VPATH		:=	${VPATH}:${ENG}/motif/mwm
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/motif/mwm
VPATH		:=	${VPATH}:${ENG}/apps/clients
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/clients
VPATH		:=	${VPATH}:${ENG}/apps/config
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/config
VPATH		:=	${VPATH}:${ENG}/apps/custom
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/custom
VPATH		:=	${VPATH}:${ENG}/apps/ims
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/ims
VPATH		:=	${VPATH}:${ENG}/apps/rte
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/rte
VPATH		:=	${VPATH}:${ENG}/apps/aixterm
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/aixterm
VPATH		:=	${VPATH}:${ENG}/apps/xdm
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/xdm
VPATH		:=	${VPATH}:${ENG}/adt/ext
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/adt/ext
VPATH		:=	${VPATH}:${ENG}/adt/imake
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/adt/imake
VPATH		:=	${VPATH}:${ENG}/fnt/fontServer
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/fnt/fontServer
VPATH		:=	${VPATH}:${ENG}/fnt/util
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/fnt/util
VPATH		:=	${VPATH}:${ENG}/Dt/rte
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/Dt/rte
VPATH		:=	${VPATH}:${ENG}/info/rte
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/info/rte
VPATH		:=	${VPATH}:${ENG}/vsm/rte
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/vsm/rte
VPATH		:=	${VPATH}:${ENG}/x_st_mgr/rte
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/x_st_mgr/rte

BFF			 =	X11.msg.${MSGLANG}

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.msg.${MSGLANG}.base.rte				\
				X11.msg.${MSGLANG}.base.common			\
				X11.msg.${MSGLANG}.motif.lib			\
				X11.msg.${MSGLANG}.motif.mwm			\
				X11.msg.${MSGLANG}.apps.clients			\
				X11.msg.${MSGLANG}.apps.config			\
				X11.msg.${MSGLANG}.apps.custom			\
				X11.msg.${MSGLANG}.apps.ims				\
				X11.msg.${MSGLANG}.apps.rte				\
				X11.msg.${MSGLANG}.apps.aixterm			\
				X11.msg.${MSGLANG}.apps.xdm				\
				X11.msg.${MSGLANG}.adt.ext				\
				X11.msg.${MSGLANG}.adt.imake			\
				X11.msg.${MSGLANG}.fnt.fontServer		\
				X11.msg.${MSGLANG}.fnt.util				\
				X11.msg.${MSGLANG}.Dt.rte				\
				X11.msg.${MSGLANG}.info.rte 			\
				X11.msg.${MSGLANG}.vsm.rte				\
				X11.msg.${MSGLANG}.x_st_mgr.rte

.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/base/rte/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/base/common/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/motif/lib/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/motif/mwm/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/clients/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/config/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/custom/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/ims/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/rte/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/aixterm/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/xdm/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/adt/ext/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/adt/imake/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/fnt/fontServer/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/fnt/util/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/Dt/rte/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/info/rte/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/vsm/rte/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/x_st_mgr/rte/packdep.mk>

.include <${RULES_MK}>
