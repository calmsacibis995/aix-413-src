# @(#)31        1.1  src/rspc/usr/lib/methods/cfgstok/Makefile, pcitok, rspc41J, 9513A_all 3/24/95 14:57:24
#
# COMPONENT_NAME: PCITOK
#
# FUNCTIONS: Makefile for IBM PCI Tokenring device config methods
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

VPATH		= ../common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon


PROGRAMS	= cfgstok

OFILES          = cfgstok.o cfgcommon.o cfgtoolsx.o chkslot.o coment.o \
		cfg_runtime_br_rspc.o  \
		cfg_cdli.o cfg_get_dvdr.o

INCFLAGS = -I${MAKETOP}/rspc/kernext/pci -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS := ${INCFLAGS} -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

ILIST		= cfgstok
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
