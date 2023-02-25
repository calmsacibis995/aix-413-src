# @(#)70  1.5  src/rspc/usr/lib/methods/cfgpctok/Makefile, pcmciatok, rspc41J, 9511A_all 3/9/95 02:45:55
#
# COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver
#
# FUNCTIONS:
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

CFLAGS          += -DPCMCIATOK -DPM_SUPPORT

VPATH		= ../common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS        = cfgpctok

OFILES          = cfgpctok.o \
		cfgcommon.o \
		cfgtoolsx.o \
		chkslot.o \
		comtok.o \
		cfg_runtime_br_rspc.o  \
		cfg_cdli.o \
		cfg_get_dvdr.o

INCFLAGS = -I${MAKETOP}/rspc/kernext/pcmcia -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS := ${INCFLAGS} -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

IMPORTS         = ${MAKETOP}/rspc/kernext/pcmcia/cs/pcmciacs.exp

ILIST           = cfgpctok
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
