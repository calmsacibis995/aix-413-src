# @(#)97        1.1  src/packages/X11/msg/en_US/fnt/util/Makefile, pkgx, pkg411, GOLD410 3/22/94 16:06:54
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

VPATH		 =	${ENG}/fnt/util
VPATH		:=	${VPATH}:${MAKETOP}/packages/X11/msg/${MSGLANG}/fnt/util

BFF			 =	X11.msg.${MSGLANG}

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.msg.${MSGLANG}.fnt.util

.include <${MAKETOP}/packages/X11/msg/${MSGLANG}/fnt/util/packdep.mk>

.include <${RULES_MK}>
