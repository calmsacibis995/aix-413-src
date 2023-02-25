# @(#)41	1.10  src/packages/devices/mca/8f70/Makefile, pkgdevcommo, pkg41J, 9519B_all 5/11/95 14:11:25
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
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
.if (${PTF_UPDATE} != "yes" )
SUBDIRS          = diag
.endif

VPATH		= rte:rte/usr:diag:diag/usr:mpqp:mpqp/usr
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/mpqp
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/mpqp/root

BFF		= devices.mca.8f70

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.8f70.rte \
		  devices.mca.8f70.mpqp \
		  devices.mca.8f70.diag

ROOT_OPTIONS	= devices.mca.8f70.mpqp

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <mpqp/packdep.mk>

.include <${RULES_MK}>
