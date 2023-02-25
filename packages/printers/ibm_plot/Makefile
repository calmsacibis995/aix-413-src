# @(#)16 1.1 src/packages/printers/ibm_plot/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:52:36
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
VPATH		:= ${VPATH}:../ibm6180:../ibm6180/rte
VPATH		:= ${VPATH}:../ibm6182:../ibm6182/rte
VPATH		:= ${VPATH}:../ibm6184:../ibm6184/rte
VPATH		:= ${VPATH}:../ibm6185-1:../ibm6185-1/rte
VPATH		:= ${VPATH}:../ibm6185-2:../ibm6185-2/rte
VPATH		:= ${VPATH}:../ibm6186:../ibm6186/rte
VPATH		:= ${VPATH}:../ibm7372:../ibm7372/rte

BFF		= printers

ILIST		= ${BFF}.ibm_plot
IDIR		= /

OPTIONS		= printers.ibm6180.rte \
		  printers.ibm6182.rte \
		  printers.ibm6184.rte \
		  printers.ibm6185-1.rte \
		  printers.ibm6185-2.rte \
		  printers.ibm6186.rte \
		  printers.ibm7372.rte

.include <../ibm6180/rte/packdep.mk>
.include <../ibm6182/rte/packdep.mk>
.include <../ibm6184/rte/packdep.mk>
.include <../ibm6185-1/rte/packdep.mk>
.include <../ibm6185-2/rte/packdep.mk>
.include <../ibm6186/rte/packdep.mk>
.include <../ibm7372/rte/packdep.mk>

.include <${RULES_MK}>

printers.ibm_plot : printers
	${CP} printers printers.ibm_plot
