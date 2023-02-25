# @(#)62	1.7  src/packages/bos/diag/rte/Makefile, pkgdiag, pkg412, 9443ct.makefile 10/27/94 23:42:25
#
# COMPONENT_NAME: PKGDIAG
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = ${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/diag/diags
VPATH		:= ${VPATH}:${MAKETOP}/bos/kernext/diagex

BFF		= bos.diag

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.diag.rte

ROOT_OPTIONS	= bos.diag.rte

.include <packdep.mk>

.include <${RULES_MK}>

root/bos.diag.rte.err.S: $${.TARGET:T}
	if [ ! -d root ] ; then ${MKDIR} root ; fi
	${CP} ${.ALLSRC} ${.TARGET}

root/bos.diag.rte.trc.S: $${.TARGET:T}
	if [ ! -d root ] ; then ${MKDIR} root ; fi
	${CP} ${.ALLSRC} ${.TARGET}
