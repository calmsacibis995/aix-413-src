# @(#)78        1.6  src/packages/dps/Makefile, pkgx, pkg411, GOLD410 5/31/94 09:09:57
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

VPATH		 =	rte
VPATH		:=	${VPATH}:samples
VPATH		:=	${VPATH}:fnt
VPATH		:=	${VPATH}:copyright

BFF			 =	dps

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	dps.rte			\
				dps.samples		\
				dps.fnt			\
				dps.copyright

.include <rte/packdep.mk>
.include <samples/packdep.mk>
.include <fnt/packdep.mk>
.include <copyright/packdep.mk>

.include <${RULES_MK}>
