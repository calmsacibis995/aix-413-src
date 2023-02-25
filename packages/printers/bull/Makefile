# @(#)09 1.1 src/packages/printers/bull/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:50:41
#
# COMPONENT_NAME: pkgdevprint
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		 = ${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:../bull200:../bull200/rte
VPATH		:= ${VPATH}:../bull201:../bull201/rte
VPATH		:= ${VPATH}:../bull411:../bull411/rte
VPATH		:= ${VPATH}:../bull413:../bull413/rte
VPATH		:= ${VPATH}:../bull422:../bull422/rte
VPATH		:= ${VPATH}:../bull451:../bull451/rte
VPATH		:= ${VPATH}:../bull454:../bull454/rte
VPATH		:= ${VPATH}:../bull721:../bull721/rte
VPATH		:= ${VPATH}:../bull815:../bull815/rte
VPATH		:= ${VPATH}:../bull825:../bull825/rte
VPATH		:= ${VPATH}:../bull922:../bull922/com:../bull922/rte
VPATH		:= ${VPATH}:../bull923:../bull923/rte
VPATH		:= ${VPATH}:../bull924:../bull924/rte
VPATH		:= ${VPATH}:../bull924N:../bull924N/rte
VPATH		:= ${VPATH}:../bull956:../bull956/rte
VPATH		:= ${VPATH}:../bull970:../bull970/com:../bull970/rte
VPATH		:= ${VPATH}:../bull1015:../bull1015/com:../bull1015/rte
VPATH		:= ${VPATH}:../bull1021:../bull1021/rte
VPATH		:= ${VPATH}:../bull1025:../bull1025/rte
VPATH		:= ${VPATH}:../bull1070:../bull1070/rte
VPATH		:= ${VPATH}:../bull1625:../bull1625/rte
VPATH		:= ${VPATH}:../bull9142:../bull9142/rte
VPATH		:= ${VPATH}:../bull9148:../bull9148/rte
VPATH		:= ${VPATH}:../bullpr88-vfu:../bullpr88-vfu/rte
VPATH		:= ${VPATH}:../bullpr88:../bullpr88/rte
VPATH		:= ${VPATH}:../bullpr90:../bullpr90/rte

BFF		= printers

ILIST		= ${BFF}.bull
IDIR		= /

OPTIONS		= printers.bull200.rte	printers.bull201.rte \
		  printers.bull411.rte	printers.bull413.rte \
		  printers.bull422.rte 	printers.bull451.rte \
		  printers.bull454.rte 	printers.bull721.rte \
		  printers.bull815.rte 	printers.bull825.rte \
		  printers.bull922.com 	printers.bull922.rte \
		  printers.bull923.rte 	printers.bull924.rte \
		  printers.bull924N.rte	printers.bull956.rte \
		  printers.bull970.com	printers.bull970.rte \
		  printers.bull1015.com	printers.bull1015.rte \
		  printers.bull1021.rte	printers.bull1025.rte \
	 	  printers.bull1070.rte	printers.bull1625.rte \
		  printers.bull9142.rte	printers.bull9148.rte \
		  printers.bullpr88-vfu.rte \
		  printers.bullpr88.rte	printers.bullpr90.rte \

.include <../bull200/rte/packdep.mk>
.include <../bull201/rte/packdep.mk>
.include <../bull411/rte/packdep.mk>
.include <../bull413/rte/packdep.mk>
.include <../bull422/rte/packdep.mk>
.include <../bull451/rte/packdep.mk>
.include <../bull454/rte/packdep.mk>
.include <../bull721/rte/packdep.mk>
.include <../bull815/rte/packdep.mk>
.include <../bull825/rte/packdep.mk>
.include <../bull922/com/packdep.mk>
.include <../bull922/rte/packdep.mk>
.include <../bull923/rte/packdep.mk>
.include <../bull924/rte/packdep.mk>
.include <../bull924N/rte/packdep.mk>
.include <../bull956/rte/packdep.mk>
.include <../bull970/com/packdep.mk>
.include <../bull970/rte/packdep.mk>
.include <../bull1015/com/packdep.mk>
.include <../bull1015/rte/packdep.mk>
.include <../bull1021/rte/packdep.mk>
.include <../bull1025/rte/packdep.mk>
.include <../bull1070/rte/packdep.mk>
.include <../bull1625/rte/packdep.mk>
.include <../bull9142/rte/packdep.mk>
.include <../bull9148/rte/packdep.mk>
.include <../bullpr88-vfu/rte/packdep.mk>
.include <../bullpr88/rte/packdep.mk>
.include <../bullpr90/rte/packdep.mk>

.include <${RULES_MK}>

printers.bull : printers
	${CP} printers printers.bull
