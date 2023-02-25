# static char sccsid[] = "@(#)94	1.2  src/rspc/usr/lbin/tty/crash/Makefile, isatty, rspc411, 9432A411a 8/8/94 14:59:31";
#
# COMPONENT_NAME: (isatty) System Extension for tty/crash
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	Make file for tty crash.
#

VPATH		= ${MAKETOP}/bos/usr/lbin/tty/crash:${MAKETOP}/rspc/kernext/tty:${MAKETOP}/bos/kernext/tty:${MAKETOP}/rspc/kernext/isa/tty

TTYDIR=/usr/lbin/tty
CRASH_PREFIX=crash-

CFLAGS	= '-DTTYDIR="$(TTYDIR)"' '-DCRASH_PREFIX="$(CRASH_PREFIX)"' \
		-DTTYDBG -DFOR_CRASH -DIN_TTYDBG -D_KERNEL
OBJECTS		= unpack.o
PROGRAMS	= crash-sf

crash-sf_OFILES	= crash-sf.o sty_db.o sf_db.o ${OBJECTS}

INCDIRS:=${INCDIRS:S;/usr/include$;/kernel/include;g}

INCFLAGS	= -I${MAKETOP}/bos/kernel -I${MAKETOP}/rspc/kernext/tty \
			-I${MAKETOP}/bos/usr/lbin/tty/crash	\
			-I${MAKETOP}/bos/kernext/tty		\
			-I${MAKETOP}/bos/kernext/tty/POWER

LIBS		= -lIN -lc -lPW
ILIST		= ${PROGRAMS}

tty_IDIR	= /usr/lib/ras/dmprtns/
IDIR		= /usr/lbin/tty/

.include <${RULES_MK}>
