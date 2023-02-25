# @(#)23	1.4  src/rspc/kernext/tty/Makefile, isatty, rspc411, GOLD410 6/28/94 13:10:58
#
#   COMPONENT_NAME: ???
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

BOZ             = ${MAKETOP}/bos/kernext/
KERNEL_EXT	= stydd
IDIR		= /usr/lib/drivers/
ILIST		= stydd
VPATH           :=${VPATH}:${BOZ}tty:${BOZ}tty/${TARGET_MACHINE}:
IMPORTS         = -bI:pse.exp sockets.exp -bI:pse_int.exp
INCFLAGS	= -I${BOZ}tty -I${BOZ}tty/${TARGET_MACHINE}

.if defined(KOFF) && ${KOFF} == "-D_KOFF"
CFLAGS	       += -DKOFF -DDEBUG -D_KERNSYS
sty_SYMS	= sty_syms.o
.else
CFLAGS	       += -D_KERNSYS
sty_SYMS	=
.endif

stydd_ENTRYPOINT	= styconfig

LIBS			= ${KERNEXT_LIBS}

stydd_COMMON	 	= common_open.o sdtropen.o swtopen.o
stydd_OFILES	 	= sty.o  sty_db.o ${sty_SYMS} ${stydd_COMMON}
stydd_IMPORTS	 	= ${IMPORTS} -bI:dbtty.exp # -bI:koff.exp
stydd_EXPORTS	 	= sty.exp

.include <${RULES_MK}>

syms ${sty_SYMS}: sty_syms.s

sty_syms.s:
	/u/dap/bin/mk_syms41 stydd >sty_syms.s

new_syms:
	${RM} -f *_syms.s *_syms.o

