# @(#)58	1.8  src/packages/devices/mca/fe92/Makefile, pkgdevcommo, pkg411, GOLD410 7/6/94 07:48:10
#
# COMPONENT_NAME: pkgdevcommo
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
VPATH		 = rte:rte/usr:diag:diag/usr:ucode:ucode/usr
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.fe92

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.fe92.rte \
		  devices.mca.fe92.diag \
		  devices.mca.fe92.ucode 

ROOT_OPTIONS	= devices.mca.fe92.rte

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <ucode/packdep.mk>

.include <${RULES_MK}>
