# @(#)90        1.3  src/rspc/kernext/isa/ide/Makefile, isaide, rspc41J 3/13/95 11:01:57
#
# COMPONENT_NAME: ISAIDE
#
# FUNCTIONS: None
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS			= ataide_errids.h \
			  root/devices.isa_sio.PNP0600.rte.err.S \
			  root/devices.isa_sio.PNP0600.rte.trc.S

KERNEL_EXT		= ataide ataidepin
IDIR			= /usr/lib/drivers/
ILIST			= ataide ataidepin

LIBS			= ${KERNEXT_LIBS}

CFLAGS			= -D_KERNEL -DPM_SUPPORT

ataide_ENTRYPOINT	= lide_config
ataide_IMPORTS		= -bI:ataidepin.exp
ataide_EXPORTS		= -bE:ataide.exp

ataidepin_IMPORTS	= -bI:ataide.exp
ataidepin_EXPORTS	= -bE:ataidepin.exp

ataide_OFILES	 	= ataidet.o
ataidepin_OFILES 	= ataideb.o

STRIP_FLAG		?= -s

LDFLAGS_KERNEXT		+= ${STRIP_FLAG}

ataide_errids.h:	devices.isa_sio.PNP0600.rte.err.S
	${CP} ${devices.isa_sio.PNP0600.rte.err.S:P} .
	${ERRTMPLTBLD} -d devices.isa_sio.PNP0600.rte.err.S -o ataide_errids.h
	${RM} -f devices.isa_sio.PNP0600.rte.err.S

root/devices.isa_sio.PNP0600.rte.err.S: devices.isa_sio.PNP0600.rte.err.S
	-${MKDIR} root
	${CP} ${devices.isa_sio.PNP0600.rte.err.S:P} \
			root/devices.isa_sio.PNP0600.rte.err.S

root/devices.isa_sio.PNP0600.rte.trc.S: devices.isa_sio.PNP0600.rte.trc.S
	-${MKDIR} root
	${CP} ${devices.isa_sio.PNP0600.rte.trc.S:P} \
			root/devices.isa_sio.PNP0600.rte.trc.S

.include <${RULES_MK}>
