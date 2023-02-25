# @(#)14 1.3 src/packages/printers/ibm_2/Makefile, pkgdevprint, pkg41J, 9517A_all 4/19/95 17:46:14
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
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = ${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:../ibm2380-2:../ibm2380-2/rte
VPATH		:= ${VPATH}:../ibm2381-2:../ibm2381-2/rte
VPATH		:= ${VPATH}:../ibm2390-2:../ibm2390-2/rte
VPATH		:= ${VPATH}:../ibm2391-2:../ibm2391-2/rte
VPATH		:= ${VPATH}:../ibm4019:../ibm4019/rte
VPATH		:= ${VPATH}:../ibm4019_AR:../ibm4019_AR/rte
VPATH		:= ${VPATH}:../ibm4019_JP:../ibm4019_JP/rte
VPATH		:= ${VPATH}:../ibm4019_KR:../ibm4019_KR/rte
VPATH		:= ${VPATH}:../ibm4029:../ibm4029/rte
VPATH		:= ${VPATH}:../ibm4029_JP:../ibm4029_JP/rte
VPATH		:= ${VPATH}:../ibm4037:../ibm4037/rte
VPATH		:= ${VPATH}:../ibm4039:../ibm4039/rte
VPATH		:= ${VPATH}:../ibm4079:../ibm4079/rte
VPATH		:= ${VPATH}:../ibm4224:../ibm4224/rte
VPATH		:= ${VPATH}:../ibm4226:../ibm4226/rte
VPATH		:= ${VPATH}:../ibm4234:../ibm4234/rte
VPATH		:= ${VPATH}:../ibm4234_AR:../ibm4234_AR/rte:
VPATH		:= ${VPATH}:../ibm6252:../ibm6252/rte
VPATH		:= ${VPATH}:../ibm6262:../ibm6262/rte
VPATH		:= ${VPATH}:../ibm3112:../ibm3112/rte
VPATH		:= ${VPATH}:../ibm3116:../ibm3116/rte

BFF		= printers

ILIST		= ${BFF}.ibm_2
IDIR		= /

OPTIONS		= printers.ibm2380-2.rte \
		  printers.ibm2381-2.rte \
		  printers.ibm2390-2.rte \
		  printers.ibm2391-2.rte \
		  printers.ibm4019.rte \
		  printers.ibm4019_AR.rte \
		  printers.ibm4019_JP.rte \
		  printers.ibm4019_KR.rte \
		  printers.ibm4029.rte \
		  printers.ibm4029_JP.rte \
		  printers.ibm4037.rte \
		  printers.ibm4039.rte \
		  printers.ibm4079.rte \
		  printers.ibm4224.rte \
		  printers.ibm4226.rte \
		  printers.ibm4234.rte \
		  printers.ibm4234_AR.rte \
		  printers.ibm6252.rte \
		  printers.ibm6262.rte \
		  printers.ibm3112.rte \
		  printers.ibm3116.rte

.include <../ibm2380-2/rte/packdep.mk>
.include <../ibm2381-2/rte/packdep.mk>
.include <../ibm2390-2/rte/packdep.mk>
.include <../ibm2391-2/rte/packdep.mk>
.include <../ibm4019/rte/packdep.mk>
.include <../ibm4019_AR/rte/packdep.mk>
.include <../ibm4019_JP/rte/packdep.mk>
.include <../ibm4019_KR/rte/packdep.mk>
.include <../ibm4029/rte/packdep.mk>
.include <../ibm4029_JP/rte/packdep.mk>
.include <../ibm4037/rte/packdep.mk>
.include <../ibm4039/rte/packdep.mk>
.include <../ibm4079/rte/packdep.mk>
.include <../ibm4224/rte/packdep.mk>
.include <../ibm4226/rte/packdep.mk>
.include <../ibm4234/rte/packdep.mk>
.include <../ibm4234_AR/rte/packdep.mk>
.include <../ibm6252/rte/packdep.mk>
.include <../ibm6262/rte/packdep.mk>
.include <../ibm3112/rte/packdep.mk>
.include <../ibm3116/rte/packdep.mk>

.include <${RULES_MK}>

printers.ibm_2 : printers
	${CP} printers printers.ibm_2
