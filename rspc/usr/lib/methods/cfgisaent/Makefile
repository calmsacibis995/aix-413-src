# @(#)56        1.3  src/rspc/usr/lib/methods/cfgisaent/Makefile, isaent, rspc41J, 9509A_all 2/14/95 10:49:12
#
# COMPONENT_NAME: isaent - IBM ISA Ethernet device driver
#
# FUNCTIONS: Makefile for IBM ISA Ethernet device config methods
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

VPATH		= ../common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon


PROGRAMS	= cfgisaent

OFILES          = cfgisaent.o cfgcommon.o cfgtoolsx.o chkslot.o coment.o \
		cfg_runtime_br_rspc.o  \
		cfg_cdli.o cfg_get_dvdr.o

INCFLAGS = -I${MAKETOP}/rspc/kernext/isa -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS := ${INCFLAGS} -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

ILIST		= cfgisaent
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
