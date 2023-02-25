# @(#)83        1.9  src/packages/devices/mca/8d77/Makefile, pkgdevbase, pkg411, GOLD410 3/9/94 11:15:54
#
# COMPONENT_NAME: pkgdevbase
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
VPATH		 = com:rte:diag:ucode
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.8d77

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8d77.rte \
		  devices.mca.8d77.com \
		  devices.mca.8d77.diag \
		  devices.mca.8d77.ucode


.include <com/packdep.mk>
.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <ucode/packdep.mk>

.include <${RULES_MK}>
