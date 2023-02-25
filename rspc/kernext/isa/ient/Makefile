#  @(#)67       1.5.1.2  src/rspc/kernext/isa/ient/Makefile, isaent, rspc41J, 9516A_all 4/13/95 16:11:46 
#
# COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
#
# FUNCTIONS:      Makefile for Ethernet Device Driver
#
# ORIGINS:        27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

OTHERS		= ient_errids.h \
		  root/devices.isa.ethernet.rte.err.S  \
		  root/devices.isa.ethernet.rte.trc.S

KERNEL_EXT	= ientdd

#CFLAGS         += -DDEBUG -qlist -qsource
#STRIP_FLAG	=

IDIR		= /usr/lib/drivers/isa/

ILIST		= ientdd

LIBS		= ${KERNEXT_LIBS}

ientdd_ENTRYPOINT = ient_config

ientdd_OFILES = ient_cfg.o ient_open.o ient_init.o ient_close.o ient_util.o \
		ient_misc.o ient_intr.o ient_err.o ient_ioctl.o ient_rw.o \
		ient_pmgmt.o

testset.o_INCFLAGS      = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

ient_errids.h: isaeth.err.S
		${CP} ${isaeth.err.S:P} .
		${ERRTMPLTBLD} -d isaeth.err.S -o ${.TARGET}
		${RM} -f isaeth.err.S

root/devices.isa.ethernet.rte.err.S: isaeth.err.S
		-${MKDIR} -p root
		${CP} ${isaeth.err.S:P} ${.TARGET}

root/devices.isa.ethernet.rte.trc.S: isaeth.trc.S
		-${MKDIR} -p root
		${CP} ${isaeth.trc.S:P} ${.TARGET}

.include <${RULES_MK}>
