# @(#)74  1.2  src/rspc/kernext/pcmcia/ent/Makefile, pcmciaent, rspc41J, 9511A_all 3/9/95 03:01:52
#  @(#)67       1.5.1.1  src/rspc/kernext/isa/ient/Makefile, isaent, rspc41J 1/6/95 15:46:44 
#
# COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet Device Driver
#
# FUNCTIONS:      Makefile for Ethernet Device Driver
#
# ORIGINS:        27
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

OTHERS		= ient_errids.h \
		  root/devices.pcmcia.ethernet.rte.err.S  \
		  root/devices.pcmcia.ethernet.rte.trc.S

VPATH           +=:${MAKETOP}/rspc/kernext/isa/ient:

KERNEL_EXT	= pcentdd

CFLAGS		+= -DPCMCIAENT -DPM_SUPPORT

IDIR		= /usr/lib/drivers/pcmcia/

ILIST		= pcentdd

LIBS		= ${KERNEXT_LIBS}

IMPORTS         = ${MAKETOP}/rspc/kernext/pcmcia/cs/pcmciacs.exp

pcentdd_ENTRYPOINT = ient_config

pcentdd_OFILES = ient_cfg.o ient_open.o ient_init.o ient_close.o ient_util.o \
		ient_misc.o ient_intr.o ient_err.o ient_ioctl.o ient_rw.o \
		ient_pcmcia.o

testset.o_INCFLAGS      = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

ient_errids.h: isaeth.err.S
		${CP} ${isaeth.err.S:P} .
		${ERRTMPLTBLD} -d isaeth.err.S -o ient_errids.h
		${RM} -f isaeth.err.S

root/devices.pcmcia.ethernet.rte.err.S: isaeth.err.S
		-${MKDIR} root
		${CP} ${isaeth.err.S:P} root/devices.pcmcia.ethernet.rte.err.S

root/devices.pcmcia.ethernet.rte.trc.S: isaeth.trc.S
		-${MKDIR} root
		${CP} ${isaeth.trc.S:P} root/devices.pcmcia.ethernet.rte.trc.S

.include <${RULES_MK}>
