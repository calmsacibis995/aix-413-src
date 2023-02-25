# @(#)13 1.1 src/packages/printers/ibm_1/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:51:41
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
VPATH		:= ${VPATH}:../ibm2380:../ibm2380/rte
VPATH		:= ${VPATH}:../ibm2381:../ibm2381/rte
VPATH		:= ${VPATH}:../ibm2390:../ibm2390/rte
VPATH		:= ${VPATH}:../ibm2391:../ibm2391/rte
VPATH		:= ${VPATH}:../ibm3812-2:../ibm3812-2/rte
VPATH		:= ${VPATH}:../ibm3816:../ibm3816/rte
VPATH		:= ${VPATH}:../ibm4070:../ibm4070/rte
VPATH		:= ${VPATH}:../ibm4072:../ibm4072/rte
VPATH		:= ${VPATH}:../ibm4076:../ibm4076/rte
VPATH		:= ${VPATH}:../ibm4201-2:../ibm4201-2/rte
VPATH		:= ${VPATH}:../ibm4201-3:../ibm4201-3/rte
VPATH		:= ${VPATH}:../ibm4202-2:../ibm4202-2/rte
VPATH		:= ${VPATH}:../ibm4202-3:../ibm4202-3/rte
VPATH		:= ${VPATH}:../ibm4207-2:../ibm4207-2/rte
VPATH		:= ${VPATH}:../ibm4208-2:../ibm4208-2/rte
VPATH		:= ${VPATH}:../ibm4212:../ibm4212/rte
VPATH		:= ${VPATH}:../ibm4216-31:../ibm4216-31/rte
VPATH		:= ${VPATH}:../ibm5202:../ibm5202/rte
VPATH		:= ${VPATH}:../ibm5204:../ibm5204/rte

BFF		= printers

ILIST		= ${BFF}.ibm_1
IDIR		= /

OPTIONS		= printers.ibm2380.rte \
		  printers.ibm2381.rte \
		  printers.ibm2390.rte \
		  printers.ibm2391.rte \
		  printers.ibm3812-2.rte \
		  printers.ibm3816.rte \
		  printers.ibm4070.rte \
		  printers.ibm4072.rte \
		  printers.ibm4076.rte \
		  printers.ibm4201-2.rte \
		  printers.ibm4201-3.rte \
		  printers.ibm4202-2.rte \
		  printers.ibm4202-3.rte \
		  printers.ibm4207-2.rte \
		  printers.ibm4208-2.rte \
		  printers.ibm4212.rte \
		  printers.ibm4216-31.rte \
		  printers.ibm5202.rte \
		  printers.ibm5204.rte

.include <../ibm2380/rte/packdep.mk>
.include <../ibm2381/rte/packdep.mk>
.include <../ibm2390/rte/packdep.mk>
.include <../ibm2391/rte/packdep.mk>
.include <../ibm3812-2/rte/packdep.mk>
.include <../ibm3816/rte/packdep.mk>
.include <../ibm4070/rte/packdep.mk>
.include <../ibm4072/rte/packdep.mk>
.include <../ibm4076/rte/packdep.mk>
.include <../ibm4201-2/rte/packdep.mk>
.include <../ibm4201-3/rte/packdep.mk>
.include <../ibm4202-2/rte/packdep.mk>
.include <../ibm4202-3/rte/packdep.mk>
.include <../ibm4207-2/rte/packdep.mk>
.include <../ibm4208-2/rte/packdep.mk>
.include <../ibm4212/rte/packdep.mk>
.include <../ibm4216-31/rte/packdep.mk>
.include <../ibm5202/rte/packdep.mk>
.include <../ibm5204/rte/packdep.mk>

.include <${RULES_MK}>

printers.ibm_1 : printers
	${CP} printers printers.ibm_1
