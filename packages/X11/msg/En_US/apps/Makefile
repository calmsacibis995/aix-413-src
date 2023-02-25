# @(#)02        1.10  src/packages/X11/msg/en_US/apps/Makefile, pkgx, pkg411, 9439A411b 9/26/94 14:59:18
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

VPATH		 =	${ENG}/apps/clients
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

BFF			 =	X11.msg.${MSGLANG}

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.msg.${MSGLANG}.apps.clients			\
				X11.msg.${MSGLANG}.apps.config			\
				X11.msg.${MSGLANG}.apps.custom			\
				X11.msg.${MSGLANG}.apps.ims				\
				X11.msg.${MSGLANG}.apps.rte				\
				X11.msg.${MSGLANG}.apps.aixterm

.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/clients/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/config/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/custom/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/ims/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/rte/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/apps/aixterm/packdep.mk>

.include <${RULES_MK}>
