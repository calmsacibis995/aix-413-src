# %Z%%M%  %I%  %W% %G% %U%
# @(#)59	1.4  src/rspc/kernext/isa/itok/Makefile, isatok, rspc41J 7/5/94 14:37:15
#
# COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
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

OTHERS          = itok_errids.h \
                  root/devices.pcmcia.tokenring.rte.err.S  \
                  root/devices.pcmcia.tokenring.rte.trc.S

CFLAGS		+= -DPCMCIATOK -DPM_SUPPORT

VPATH           +=:${MAKETOP}/rspc/kernext/isa/itok:

IMPORTS		= ${MAKETOP}/rspc/kernext/pcmcia/cs/pcmciacs.exp

INCFLAGS        = -I${MAKETOP}/rspc/kernext/isa/itok -I${MAKETOP}/rspc/kernext/inc

KERNEL_EXT	= pctokdd

IDIR		= /usr/lib/drivers/pcmcia/
ILIST		= pctokdd	

LIBS		= ${KERNEXT_LIBS}

pctokdd_ENTRYPOINT	= tokconfig

pctokdd_OFILES	= itok_cfg.o itok_init.o itok_open.o itok_close.o itok_dump.o \
		itok_misc.o itok_rw.o itok_ioctl.o itok_intr.o itok_debug.o  \
		itok_pcmcia.o
		 

testset.o_INCFLAGS      = -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}

itok_errids.h: isatok.err.S
		${CP} ${isatok.err.S:P} .
		${ERRTMPLTBLD} -d isatok.err.S -o itok_errids.h
		${RM} -f isatok.err.S

root/devices.pcmcia.tokenring.rte.err.S: isatok.err.S
		-${MKDIR} root
		${CP} ${isatok.err.S:P} root/devices.pcmcia.tokenring.rte.err.S

root/devices.pcmcia.tokenring.rte.trc.S: isatok.trc.S
		-${MKDIR} root
		${CP} ${isatok.trc.S:P} root/devices.pcmcia.tokenring.rte.trc.S


.include <${RULES_MK}>
