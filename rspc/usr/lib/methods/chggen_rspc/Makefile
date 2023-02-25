# @(#)28        1.1  src/rspc/usr/lib/methods/chggen_rspc/Makefile, rspccfg, rspc411, GOLD410 6/29/94 08:45:03
# COMPONENT_NAME: rspccfg
#
# FUNCTIONS:
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

VPATH           += ${MAKETOP}/bos/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/rspc/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/chggen

PROGRAMS	= chggen_rspc

OFILES		= chgdevice.o cfgtools.o loc_rspc.o chggen.o

INCFLAGS        += -I${MAKETOP}/bos/usr/lib/methods/common -I../common

ILIST		= chggen_rspc
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
