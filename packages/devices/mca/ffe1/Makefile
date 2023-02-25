# @(#)94        1.12  src/packages/devices/mca/ffe1/Makefile, pkgdevtty, pkg41J, 174218.pkg 3/23/95 17:43:23
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
VPATH		:= ${VPATH}:${MAKETOP}/bos/kernext/mca_async

BFF		= devices.mca.ffe1

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.ffe1.diag \
		  devices.mca.ffe1.rte \
		  devices.mca.ffe1.ucode

ROOT_OPTIONS	= devices.mca.ffe1.rte

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <ucode/packdep.mk>

.include <${RULES_MK}>

root/devices.mca.ffe1.rte.err.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}


root/devices.mca.ffe1.rte.trc.err.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}

