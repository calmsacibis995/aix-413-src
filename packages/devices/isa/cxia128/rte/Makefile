# @(#)13        1.7  src/packages/devices/mca/ffe1/rte/Makefile, pkgdevtty, pkg412, 9443ct.makefile 10/27/94 23:48:46
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
VPATH		= ${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/kernext/isa_async:${MAKETOP}/bos/kernext/mca_async:

BFF		= devices.isa.cxia128

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.isa.cxia128.rte

ROOT_OPTIONS	= devices.isa.cxia128.rte

.include <packdep.mk>

.include <${RULES_MK}>


root/devices.mca.ffe1.rte.err.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}

root/devices.mca.ffe1.rte.trc.S: $${.TARGET:T}
	-${MKDIR} root
	${CP} ${.ALLSRC} ${.TARGET}
