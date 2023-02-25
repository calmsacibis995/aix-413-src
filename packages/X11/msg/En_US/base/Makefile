# @(#)85        1.7  src/packages/X11/msg/en_US/base/Makefile, pkgx, pkg411, 9433B411a 8/18/94 09:35:59
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

BFF			 =	X11.msg.${MSGLANG}

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.msg.${MSGLANG}.base.rte	\
				X11.msg.${MSGLANG}.base.common

.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/base/rte/packdep.mk>
.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/base/common/packdep.mk>

.include <${RULES_MK}>
