# @(#)22        1.1  src/packages/printers/lex/Makefile, pkgdevprint, pkg41J, 9512A_all 3/15/95 19:16:41
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

VPATH		 = ${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:../lex4039+:../lex4039+/rte
VPATH		:= ${VPATH}:../lex4047:../lex4047/rte
VPATH		:= ${VPATH}:../lex4049:../lex4049/rte
VPATH		:= ${VPATH}:../lex4076-2c:../lex4076-2c/rte
VPATH		:= ${VPATH}:../lex4079+:../lex4079+/rte

BFF		= printers

ILIST		= ${BFF}.lex
IDIR		= /

OPTIONS		= printers.lex4039+.rte \
		  printers.lex4047.rte \
		  printers.lex4049.rte \
		  printers.lex4076-2c.rte \
		  printers.lex4079+.rte

.include <../lex4039+/rte/packdep.mk>
.include <../lex4047/rte/packdep.mk>
.include <../lex4049/rte/packdep.mk>
.include <../lex4076-2c/rte/packdep.mk>
.include <../lex4079+/rte/packdep.mk>

.include <${RULES_MK}>

printers.lex : printers
	${CP} printers printers.lex
