# @(#)01        1.2  src/rspc/kernext/isa/ppa/Makefile, isappa, rspc41J, 9510A 3/6/95 23:22:01
# COMPONENT_NAME: ISAPPA
#
# FUNCTIONS:
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

VPATH	+=:${TARGET_MACHINE}
LOCALCFLAGS     = -D_PP_ISA_BUS -DPM_PHASE2 -qarch=ppc

KERNEL_EXT	= isa_ppdd isa_ppddp
IDIR		= /usr/lib/drivers/isa/
ILIST		= isa_ppdd isa_ppddp

isa_ppdd_ENTRYPOINT 	= ppconfig
isa_ppdd_IMPORTS	= -bI:isa_ppddp.exp
isa_ppdd_EXPORTS 	= -bE:isa_ppdd.exp

isa_ppddp_IMPORTS	= -bI:isa_ppdd.exp
isa_ppddp_EXPORTS	= -bE:isa_ppddp.exp

LIBS = $(KERNEXT_LIBS)

isa_ppdd_OFILES = \
	ppconfig.o ppopen.o ppclose.o ppread.o \
	ppwrite.o ppioctl.o ppddt.o ppddm_top.o \
	prnformat.o

isa_ppddp_OFILES = \
	ppddm_bot.o pppio_bot.o ppddb.o pppm_handler.o

.include <${RULES_MK}>
