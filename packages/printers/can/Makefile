# @(#)10 1.1 src/packages/printers/can/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:50:57
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
VPATH		:= ${VPATH}:../canlbp:../canlbp/rte
VPATH		:= ${VPATH}:../canlbp-A404PS:../canlbp-A404PS/rte
VPATH		:= ${VPATH}:../canlbp-B406G:../canlbp-B406G/rte

BFF		= printers

ILIST		= ${BFF}.can
IDIR		= /

OPTIONS		= printers.canlbp-A404PS.rte \
		  printers.canlbp-B406G.rte \
		  printers.canlbp.rte

.include <../canlbp-A404PS/rte/packdep.mk>
.include <../canlbp-B406G/rte/packdep.mk>
.include <../canlbp/rte/packdep.mk>

.include <${RULES_MK}>

printers.can : printers
	${CP} printers printers.can
