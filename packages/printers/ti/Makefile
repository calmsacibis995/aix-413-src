# @(#)20 1.1 src/packages/printers/ti/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:53:21
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
VPATH		:= ${VPATH}:../ti2115:../ti2115/rte

BFF		= printers

ILIST		= ${BFF}.ti
IDIR		= /

OPTIONS		= printers.ti2115.rte

.include <../ti2115/rte/packdep.mk>

.include <${RULES_MK}>

printers.ti : printers
	${CP} printers printers.ti
