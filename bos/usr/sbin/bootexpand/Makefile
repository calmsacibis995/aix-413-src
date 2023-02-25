# @(#)60        1.8  src/bos/usr/sbin/bootexpand/Makefile, bosboot, bos41J, 9516A_all 4/18/95 16:27:44
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

BINARIES	= yes

CFLAGS		=-qarch=com -qdebug=useabs
INCFLAGS	=-I${MAKETOP}/bos/kernel/sys
LDFLAGS		=-bR:${.TARGET}.map -bdbg:loadabs -estart -T0 -D-1 -bn
OFILES		= expand.o expndkro.o millicode_boot.o start.o cache_ppc.o\
		  cache_pwr.o
OTHERS		= bootexpand

start.o_INCFLAGS = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}
expand.o_INCFLAGS = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}
cache_ppc.o_INCFLAGS = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}
cache_pwr.o_INCFLAGS = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

LIBS		= -lcsys

ILIST		= bootexpand
IDIR		= /usr/sbin/

.include <${RULES_MK}>

bootexpand:	${OFILES}
	${RM} -f ${.TARGET}.map
	${LD} ${_LDFLAGS_} -o ${.TARGET} ${OFILES} ${_LIBS_}
