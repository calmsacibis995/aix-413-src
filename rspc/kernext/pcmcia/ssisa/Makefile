# @(#)70	1.3  src/rspc/kernext/pcmcia/ssisa/Makefile, pcmciaker, rspc41J, 9509A_all 2/20/95 17:36:36
#
# COMPONENT_NAME: PCMCIAKER
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS		= pcmcia_errids.h \
			root/devices.isa.pcmcia.rte.err.S \
			root/devices.isa.pcmcia.rte.trc.S

KERNEL_EXT	= pcmciass pcmciasspin

#CFLAGS		+= -DDEBUG
#CFLAGS         += -qsource

IDIR		= /usr/lib/drivers/isa/
ILIST		= pcmciass pcmciasspin

LIBS		= ${KERNEXT_LIBS}

# INCFLAGS = ${MAKETOP}export/power/kernel/include

pcmciass_ENTRYPOINT	= pcmcia_config

pcmciass_IMPORTS    = -bI:pcmciaddpin.exp
pcmciasspin_EXPORTS = -bE:pcmciaddpin.exp

pcmciass_OFILES    = pcm_dd.o  pcm_ss.o 
pcmciasspin_OFILES = pcm_pin.o pcm_io.o pcm_misc.o

pcmcia_errids.h: pcm.err.S
		${CP} ${pcm.err.S:P} .
		${ERRTMPLTBLD} -d pcm.err.S -o pcmcia_errids.h
		${RM} -f pcm.err.S

root/devices.isa.pcmcia.rte.err.S: pcm.err.S
		-${MKDIR} root
		${CP} ${pcm.err.S:P} root/devices.isa.pcmcia.rte.err.S

root/devices.isa.pcmcia.rte.trc.S: pcm.trc.S
		-${MKDIR} root
		${CP} ${pcm.trc.S:P} root/devices.isa.pcmcia.rte.trc.S

.include <${RULES_MK}>
