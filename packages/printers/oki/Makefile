# @(#)17 1.1 src/packages/printers/oki/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:52:48
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
VPATH		:= ${VPATH}:../oki801ps:../oki801ps/rte

BFF		= printers

ILIST		= ${BFF}.oki
IDIR		= /

OPTIONS		= printers.oki801ps.rte

.include <../oki801ps/rte/packdep.mk>

.include <${RULES_MK}>

printers.oki : printers
	${CP} printers printers.oki
