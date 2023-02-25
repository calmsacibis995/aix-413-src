# @(#)83        1.5  src/packages/bos/sysmgt/trace/Makefile, pkgtrc, pkg412, 9443ct.makefile 10/27/94 23:45:48
#
# COMPONENT_NAME: pkgtrc
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ${MAKETOP}/bos/usr/sbin/trace:${MAKETOP}/bos/usr/bin/trcrpt:root

BFF		= bos.sysmgt


ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.sysmgt.trace
ROOT_OPTIONS	= bos.sysmgt.trace

.include <packdep.mk>

.include <${RULES_MK}>

root/bos.sysmgt.trace.trc.S: $${.TARGET:T}
		-${MKDIR} root
		${CP} ${.ALLSRC} ${.TARGET}
