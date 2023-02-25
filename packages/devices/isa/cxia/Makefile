# static char sccsid[] = "@(#)02	1.3  src/packages/devices/isa/cxia/Makefile, pkgdevtty, pkg41J, 9514A_all 3/30/95 17:07:28";
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
VPATH		:= ${VPATH}:${MAKETOP}/bos/kernext/isa_async

BFF		= devices.isa.cxia

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.isa.cxia.rte \
			devices.isa.cxia.ucode \
			devices.isa.cxia.diag

ROOT_OPTIONS	= devices.isa.cxia.rte

.include <rte/packdep.mk>
.include <ucode/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>

root/devices.isa.cxia.rte.err.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}


root/devices.isa.cxia.rte.trc.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}

