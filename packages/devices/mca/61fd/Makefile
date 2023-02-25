# @(#)33        1.4  src/packages/devices/mca/61fd/Makefile, pkgdevtty, pkg411, GOLD410 2/25/94 09:31:43
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
VPATH		= rte:diag
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.61fd

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.61fd.rte \
		  devices.mca.61fd.diag

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>

