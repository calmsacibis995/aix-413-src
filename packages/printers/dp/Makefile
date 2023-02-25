# @(#)11 1.1 src/packages/printers/dp/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:51:09
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
VPATH		:= ${VPATH}:../dp2000:../dp2000/rte
VPATH		:= ${VPATH}:../dp2665:../dp2665/rte

BFF		= printers

ILIST		= ${BFF}.dp
IDIR		= /

OPTIONS		= printers.dp2000.rte	printers.dp2665.rte 

.include <../dp2000/rte/packdep.mk>
.include <../dp2665/rte/packdep.mk>

.include <${RULES_MK}>

printers.dp : printers
	${CP} printers printers.dp
