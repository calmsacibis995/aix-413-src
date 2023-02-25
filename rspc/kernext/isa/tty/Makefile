# @(#)63	1.5  src/rspc/kernext/isa/tty/Makefile, isatty, rspc41J, 9512A_all 3/20/95 20:42:07
#
#   COMPONENT_NAME: isatty
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

BOZ             = ${MAKETOP}/bos/kernext/
KERNEL_EXT	= rsdd_rspc
IDIR		= /usr/lib/drivers/isa/
ILIST		= rsdd_rspc
VPATH           :=${VPATH}:${TARGET_MACHINE}:${BOZ}tty:${BOZ}tty/${TARGET_MACHINE}:
VPATH		:=${VPATH}:../../tty
IMPORTS         = -bI:pse.exp sockets.exp -bI:pse_int.exp
INCFLAGS	= -I../../tty -I${BOZ}tty -I${BOZ}tty/${TARGET_MACHINE}

.if defined(KOFF) && ${KOFF} == "-D_KOFF"
CFLAGS	       += -DKOFF -DDEBUG -D_KERNSYS -DPM_SUPPORT
sf_SYMS		= sf_syms.o
.else
CFLAGS	       += -D_KERNSYS -DPM_SUPPORT
sf_SYMS		= 
.endif

rsdd_rspc_ENTRYPOINT	= sf_config

LIBS			= ${KERNEXT_LIBS}

rsdd_rspc_OFILES	= sf.o sf_db.o ${sf_SYMS} sf_pm.o
rsdd_rspc_IMPORTS	= ${IMPORTS} -bI:sty.exp -bI:dbtty.exp

.include <${RULES_MK}>

syms ${sf_SYMS}: sf_syms.s

sf_syms.s:
	/u/dap/bin/mk_syms41 rsdd_rspc >sf_syms.s

new_syms:
	${RM} -f *_syms.s *_syms.o

