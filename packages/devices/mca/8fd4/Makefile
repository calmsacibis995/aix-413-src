# @(#)50        1.6  src/packages/devices/mca/8fd4/Makefile, pkgdevbase, pkg411, GOLD410 7/5/94 17:05:33
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
VPATH		 = rte:diag:ucode
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/POWER

BFF		= devices.mca.8fd4

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8fd4.rte
#		  devices.mca.8fd4.diag \
#		  devices.mca.8fd4.ucode

ROOT_OPTIONS	= devices.mca.8fd4.rte

.include <rte/packdep.mk>
#.include <diag/packdep.mk>
#.include <ucode/packdep.mk>

.include <${RULES_MK}>
