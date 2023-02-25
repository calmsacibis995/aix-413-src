# @(#)45	1.2  src/rspc/kernext/pcmcia/tty/Makefile, pcmciafax, rspc41J, 9512A_all 3/21/95 00:06:40
#
#   COMPONENT_NAME: pcmciafax
#
#   FUNCTIONS:
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
KERNEL_EXT	= pcrsdd
IDIR		= /usr/lib/drivers/pcmcia/
ILIST		= pcrsdd
VPATH           :=${VPATH}:${TARGET_MACHINE}:${BOZ}tty:${BOZ}tty/${TARGET_MACHINE}:
VPATH		:=${VPATH}:../../tty:${MAKETOP}rspc/kernext/isa/tty
IMPORTS         = -bI:pse.exp sockets.exp -bI:pse_int.exp
INCFLAGS	= -I../../tty -I${BOZ}tty -I${BOZ}tty/${TARGET_MACHINE} -I${MAKETOP}rspc/kernext/isa/tty

.if defined(KOFF) && ${KOFF} == "-D_KOFF"
CFLAGS	       += -DKOFF -DDEBUG -D_KERNSYS -DPCMCIA -DPM_SUPPORT
.else
CFLAGS	       += -D_KERNSYS -DPCMCIA -DPM_SUPPORT
.endif

pcrsdd_ENTRYPOINT	= sf_config

LIBS			= ${KERNEXT_LIBS}

pcrsdd_OFILES	= sf.o sf_db.o sf_pcmcia.o sf_pm.o
pcrsdd_IMPORTS	= ${IMPORTS} -bI:sty.exp -bI:dbtty.exp \
                  ${MAKETOP}/rspc/kernext/pcmcia/cs/pcmciacs.exp

.include <${RULES_MK}>

