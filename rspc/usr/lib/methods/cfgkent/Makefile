# @(#)70        1.2  src/rspc/usr/lib/methods/cfgkent/Makefile, pcient, rspc41J 3/3/95 10:21:28
#
# COMPONENT_NAME: PCIENT
#
# FUNCTIONS: Makefile for IBM PCI Ethernet device config methods
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
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon


PROGRAMS	= cfgkent

OFILES          = cfgkent.o cfgcommon.o cfgtoolsx.o chkslot.o coment.o \
		cfg_runtime_br_rspc.o  \
		cfg_cdli.o cfg_get_dvdr.o

INCFLAGS = -I${MAKETOP}/rspc/kernext/pci -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS := ${INCFLAGS} -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

ILIST		= cfgkent
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
