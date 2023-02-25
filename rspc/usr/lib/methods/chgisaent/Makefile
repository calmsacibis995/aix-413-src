# @(#)50        1.1  src/rspc/usr/lib/methods/chgisaent/Makefile, isaent, rspc411, GOLD410 6/27/94 15:03:50
#
# COMPONENT_NAME: isaent - IBM ISA Ethernet device driver
#
# FUNCTIONS: Makefile for IBM ISA Ethernet device change method
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

VPATH		= ../common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/chgent

PROGRAMS	= chgisaent

OFILES          = chgdevice.o cfgtoolsx.o loc_rspc.o chgent.o coment.o

INCFLAGS = -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS := ${INCFLAGS} -I${MAKETOP}/bos/kernext

ILIST		= chgisaent
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
