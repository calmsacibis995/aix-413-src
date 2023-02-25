# @(#)77        1.2  src/rspc/kernext/rspcios/Makefile, rspcios, rspc411, GOLD410 5/17/94 08:52:59
#
# COMPONENT_NAME:	SYSIOS
#
# FUNCTIONS:		RSPC I/O Subsystem Kernel Extension
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

INCFLAGS     += -I${MAKETOP}bos/kernel/ml/${TARGET_MACHINE}
LOCALCFLAGS	= -qarch=ppc -qmaxmem=23000 -D_RSPC -D_RSPC_UP_PCI
SUBDIRS		= 

KERNEL_EXT	= rspcios 
IDIR		= /usr/lib/drivers/isa/
ILIST		= rspcios

rspcios_ENTRYPOINT	= rspcios_init
rspcios_IMPORTS		= 
rspcios_EXPORTS		= 

LIBS		= ${KERNEXT_LIBS}

rspcios_OFILES = rspcios.o 

STRIP_FLAG ?= -s

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
