# @(#)18 1.1 src/packages/printers/pt/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:53:00
#
# COMPONENT_NAME: pkgdevprint
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = ${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:../p9012:../p9012/rte

BFF		= printers

ILIST		= ${BFF}.pt
IDIR		= /

OPTIONS		= printers.p9012.rte

.include <../p9012/rte/packdep.mk>

.include <${RULES_MK}>

printers.pt : printers
	${CP} printers printers.pt
