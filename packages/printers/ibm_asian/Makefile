# @(#)15 1.1 src/packages/printers/ibm_asian/Makefile, pkgdevprint, pkg411, GOLD410 5/31/94 19:52:10
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
VPATH		:= ${VPATH}:../ibm4208-502:../ibm4208-502/rte
VPATH		:= ${VPATH}:../ibm4216-510:../ibm4216-510/rte
VPATH		:= ${VPATH}:../ibm5327:../ibm5327/rte
VPATH		:= ${VPATH}:../ibm5572:../ibm5572/rte
VPATH		:= ${VPATH}:../ibm5573:../ibm5573/rte
VPATH		:= ${VPATH}:../ibm5575:../ibm5575/com
VPATH		:= ${VPATH}:../ibm5575_JP:../ibm5575_JP/rte
VPATH		:= ${VPATH}:../ibm5575_KR:../ibm5575_KR/rte
VPATH		:= ${VPATH}:../ibm5575_TW:../ibm5575_TW/rte
VPATH		:= ${VPATH}:../ibm5577:../ibm5577/com
VPATH		:= ${VPATH}:../ibm5577_JP:../ibm5577_JP/rte
VPATH		:= ${VPATH}:../ibm5577_KR:../ibm5577_KR/rte
VPATH		:= ${VPATH}:../ibm5577_TW:../ibm5577_TW/rte
VPATH		:= ${VPATH}:../ibm5579:../ibm5579/rte
VPATH		:= ${VPATH}:../ibm5584:../ibm5584/rte
VPATH		:= ${VPATH}:../ibm5585:../ibm5585/com
VPATH		:= ${VPATH}:../ibm5585_JP:../ibm5585_JP/rte
VPATH		:= ${VPATH}:../ibm5585_TW:../ibm5585_TW/rte
VPATH		:= ${VPATH}:../ibm5587:../ibm5587/rte
VPATH		:= ${VPATH}:../ibm5587H:../ibm5587H/rte
VPATH		:= ${VPATH}:../ibm5589:../ibm5589/rte

BFF		= printers

ILIST		= ${BFF}.ibm_asian
IDIR		= /

OPTIONS		= printers.ibm4208-502.rte \
		  printers.ibm4216-510.rte \
		  printers.ibm5327.rte \
		  printers.ibm5572.rte \
		  printers.ibm5573.rte \
		  printers.ibm5575.com \
		  printers.ibm5575_JP.rte \
		  printers.ibm5575_KR.rte \
		  printers.ibm5575_TW.rte \
		  printers.ibm5577.com \
		  printers.ibm5577_JP.rte \
		  printers.ibm5577_KR.rte \
		  printers.ibm5577_TW.rte \
		  printers.ibm5579.rte \
		  printers.ibm5584.rte \
		  printers.ibm5585.com \
		  printers.ibm5585_JP.rte \
		  printers.ibm5585_TW.rte \
		  printers.ibm5587.rte \
		  printers.ibm5587H.rte \
		  printers.ibm5589.rte

.include <../ibm4208-502/rte/packdep.mk>
.include <../ibm4216-510/rte/packdep.mk>
.include <../ibm5327/rte/packdep.mk>
.include <../ibm5572/rte/packdep.mk>
.include <../ibm5573/rte/packdep.mk>
.include <../ibm5575/com/packdep.mk>
.include <../ibm5575_JP/rte/packdep.mk>
.include <../ibm5575_KR/rte/packdep.mk>
.include <../ibm5575_TW/rte/packdep.mk>
.include <../ibm5577/com/packdep.mk>
.include <../ibm5577_JP/rte/packdep.mk>
.include <../ibm5577_KR/rte/packdep.mk>
.include <../ibm5577_TW/rte/packdep.mk>
.include <../ibm5579/rte/packdep.mk>
.include <../ibm5584/rte/packdep.mk>
.include <../ibm5585/com/packdep.mk>
.include <../ibm5585_JP/rte/packdep.mk>
.include <../ibm5585_TW/rte/packdep.mk>
.include <../ibm5587/rte/packdep.mk>
.include <../ibm5587H/rte/packdep.mk>
.include <../ibm5589/rte/packdep.mk>

.include <${RULES_MK}>

printers.ibm_asian : printers
	${CP} printers printers.ibm_asian
