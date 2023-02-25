# @(#)67	1.2  src/packages/printers/epson/Makefile, pkgdevprint, pkg41J, 9507B 2/1/95 11:01:27
#
# COMPONENT_NAME: pkgdevprint
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../epsonLQ1600K_CN/rte:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= printers

OTHERS		= printers.epson

ILIST		= ${OTHERS}
IDIR		= /

OPTIONS		= printers.epsonLQ1600K_CN.rte

.include <../epsonLQ1600K_CN/rte/packdep.mk>

.include <${RULES_MK}>

printers.epson :	printers
	${CP} ${.ALLSRC} ${.TARGET}
