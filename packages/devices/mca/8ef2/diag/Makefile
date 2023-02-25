# @(#)87        1.6  src/packages/devices/mca/8ef2/diag/Makefile, pkgdevdiag, pkg411, GOLD410 7/5/94 15:00:44
#
# COMPONENT_NAME: pkgdevdiag
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
SUBDIRS		= com

VPATH 		= com
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ientdiag
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/ientdiag/root

BFF		= devices.mca.8ef2

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8ef2.diag \
		  devices.mca.8ef2.diag.com

ROOT_OPTIONS	= devices.mca.8ef2.diag.com

.include <packdep.mk>
.include <com/packdep.mk>
.include <${RULES_MK}>
