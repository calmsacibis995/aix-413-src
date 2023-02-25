# @(#)28        1.6  src/packages/devices/mca/8f78/Makefile, pkgdevbase, pkg411, GOLD410 2/22/94 15:22:51
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
VPATH		= rte:diag:ucode
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.8f78

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8f78.rte \
		  devices.mca.8f78.diag \
		  devices.mca.8f78.ucode


.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <ucode/packdep.mk>

.include <${RULES_MK}>
