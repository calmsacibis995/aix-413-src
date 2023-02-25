# @(#)49        1.1  src/rspc/usr/lib/methods/chgisatok/Makefile, isatok, rspc411, GOLD410 6/27/94 15:03:17
#
# COMPONENT_NAME: isatok - IBM ISA Token Ring device driver
#
# FUNCTIONS: Makefile for IBM ISA Token Ring device change method
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
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/chgtok

PROGRAMS	= chgisatok

OFILES          = chgdevice.o cfgtoolsx.o loc_rspc.o chgtok.o comtok.o

INCFLAGS = -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS := ${INCFLAGS} -I${MAKETOP}/bos/kernext

ILIST		= chgisatok
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
