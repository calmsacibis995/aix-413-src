# @(#)75	1.5  src/rspc/kernext/pm/pmmd/IBM8301/Makefile, pwrsysx, rspc41J, 9516A_all 4/18/95 07:48:08
#
# COMPONENT_NAME: PWRSYSX
#
# FUNCTIONS:
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

KERNEL_EXT           = pm8301
IDIR                 = /usr/lib/drivers/isa/
ILIST                = ${KERNEL_EXT}

INCFLAGS	= -I${MAKETOP}/rspc/kernext/pm/pmmi

pm8301_ENTRYPOINT = pmmdconfig
pm8301_IMPORTS    = -bI:../../pmmi/uxpmmipin.exp -bI:../../pmmi/pmmi.exp
pm8301_EXPORTS    =

LIBS                 = ${KERNEXT_LIBS}

pm8301_OFILES   =	pmmdconfig.o pmmdentry.o pmmdintr.o pmmdmisc.o \
			planarctl.o shpcrsus.o shpcrfly.o pdppcfnc.o \
			pdkmc.o pdpic.o pdsystem.o pdrtc.o \
			pdmouse.o pdcommon.o pmtime.o

STRIP_FLAG ?= -s
CFLAGS = -D_KERNEL -D_KERNSYS
ASFLAGS    += -m 604

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
