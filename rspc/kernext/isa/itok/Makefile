# @(#)59        1.6  src/rspc/kernext/isa/itok/Makefile, isatok, rspc41J, 9522A_all 5/30/95 19:05:42
#
# COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
#
# FUNCTIONS: none
#
# ORIGINS: 27
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

OTHERS		= itok_errids.h \
		  root/devices.isa.tokenring.rte.err.S  \
		  root/devices.isa.tokenring.rte.trc.S

KERNEL_EXT	= itokdd

itokdd_ENTRYPOINT	= tokconfig

#CFLAGS		+= -DDEBUG
#STRIP_FLAG	=

${itokdd_OFILES}: itok_errids.h

itokdd_OFILES	= itok_cfg.o itok_init.o itok_open.o itok_close.o itok_dump.o \
		itok_misc.o itok_rw.o itok_ioctl.o itok_intr.o itok_debug.o \
		itok_pmgmt.o

LIBS		= ${KERNEXT_LIBS}

testset.o_INCFLAGS      = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

IDIR		= /usr/lib/drivers/isa/
ILIST		= itokdd	

.include <${RULES_MK}>

itok_errids.h: isatok.err.S
		${ERRTMPLTBLD} -d ${.ALLSRC} -o ${.TARGET}

root/devices.isa.tokenring.rte.err.S: isatok.err.S
		-${MKDIR} -p root
		${CP} ${${.ALLSRC}:P} ${.TARGET}

root/devices.isa.tokenring.rte.trc.S: isatok.trc.S
		-${MKDIR} -p root
		${CP} ${${.ALLSRC}:P} ${.TARGET}

