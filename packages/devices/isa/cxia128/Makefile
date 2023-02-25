# @(#)16	1.3  src/packages/devices/isa/cxia128/Makefile, pkgdevtty, pkg41J, 9514A_all 3/30/95 17:15:50
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
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= rte:diag:ucode
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/kernext/isa_async:${MAKETOP}/bos/kernext/mca_async:

BFF		= devices.isa.cxia128

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.isa.cxia128.rte \
			devices.isa.cxia128.ucode \
			devices.isa.cxia128.diag

ROOT_OPTIONS	= devices.isa.cxia128.rte

.include <rte/packdep.mk>
.include <ucode/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>

root/devices.mca.ffe1.rte.err.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}


root/devices.mca.ffe1.rte.trc.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}

