# static char sccsid[] = "@(#)03	1.2  src/packages/devices/isa/cxia/rte/Makefile, pkgdevtty, pkg41J, 174218.pkg 3/23/95 17:15:48";
#
# COMPONENT_NAME: pkgdevtty
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
VPATH		= ${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/kernext/isa_async

BFF		= devices.isa.cxia

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.isa.cxia.rte

ROOT_OPTIONS	= devices.isa.cxia.rte

.include <packdep.mk>

.include <${RULES_MK}>


root/devices.isa.cxia.rte.err.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}

root/devices.isa.cxia.rte.trc.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}
