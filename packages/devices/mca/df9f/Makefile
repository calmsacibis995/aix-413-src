# @(#)33        1.8  src/packages/devices/mca/df9f/Makefile, pkgdevbase, pkg411, GOLD410 2/22/94 16:16:40
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

VPATH		 = com:rte:diag
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.df9f

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.df9f.rte \
		  devices.mca.df9f.com \
		  devices.mca.df9f.diag


.include <com/packdep.mk>
.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
