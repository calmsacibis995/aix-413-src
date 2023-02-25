# @(#)87        1.7  src/packages/devices/mca/8efc/Makefile, pkgdevbase, pkg411, GOLD410 2/22/94 15:14:39
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
VPATH		 = com:diag:rte
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.8efc

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8efc.com \
		  devices.mca.8efc.rte \
		  devices.mca.8efc.diag


.include <com/packdep.mk>
.include <diag/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
