# @(#)96        1.4  src/rspc/kernext/idedisk/Makefile, idedisk, rspc41J, 173302.rspc 3/10/95 12:48:10
#
# COMPONENT_NAME: IDEDISK
#
# FUNCTIONS: None
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS			= idedisk_errids.h \
			  root/devices.ide.disk.rte.err.S \
			  root/devices.ide.disk.rte.codepoint.S \
			  root/devices.ide.disk.rte.trc.S

KERNEL_EXT		= idedisk idediskpin
IDIR			= /usr/lib/drivers/
ILIST			= idedisk idediskpin

LIBS			= ${KERNEXT_LIBS}

INCFLAGS		= -I${MAKETOP}bos/kernel/sys/POWER \
			  -I${MAKETOP}bos/kernel/sys

idedisk_ENTRYPOINT	= idedisk_config

idedisk_EXPORTS		= -bE:idedisk.exp
idediskpin_IMPORTS	= -bI:idedisk.exp

# Trace table debug requires different .exp file
:ifdef IDE_TRACE
idedisk_IMPORTS		= -bI:idediskpintr.exp
idediskpin_EXPORTS	= -bE:idediskpintr.exp
:else
idedisk_IMPORTS		= -bI:idediskpin.exp
idediskpin_EXPORTS	= -bE:idediskpin.exp
:endif

idediskpin_OFILES 	= idediskb.o
idedisk_OFILES		= idediskt.o

CFLAGS			= -D_KERNEL -DPM_SUPPORT

STRIP_FLAG		?= -s

LDFLAGS_KERNEXT		+= ${STRIP_FLAG}

idediskt.o:		idedisk_errids.h

idediskb.o:		idedisk_errids.h

idedisk_errids.h:	devices.ide.disk.rte.err.S
	${CP} ${devices.ide.disk.rte.err.S:P} .
	${ERRTMPLTBLD} -d devices.ide.disk.rte.err.S -o idedisk_errids.h
	${RM} -f devices.ide.disk.rte.err.S

root/devices.ide.disk.rte.err.S: devices.ide.disk.rte.err.S
	-${MKDIR} root
	${CP} ${devices.ide.disk.rte.err.S:P} \
			root/devices.ide.disk.rte.err.S

root/devices.ide.disk.rte.codepoint.S: devices.ide.disk.rte.codepoint.S
	-${MKDIR} root
	${CP} ${devices.ide.disk.rte.codepoint.S:P} \
			root/devices.ide.disk.rte.codepoint.S

root/devices.ide.disk.rte.trc.S: devices.ide.disk.rte.trc.S
	-${MKDIR} root
	${CP} ${devices.ide.disk.rte.trc.S:P} \
			root/devices.ide.disk.rte.trc.S

.include <${RULES_MK}>
