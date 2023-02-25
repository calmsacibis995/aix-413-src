# @(#)94  1.4  src/rspc/usr/lib/methods/cfgpcent/Makefile, pcmciaent, rspc41J, 9511A_all 3/9/95 03:10:15
#
# COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet device driver
#
# FUNCTIONS: Makefile for IBM PCMCIA Ethernet device config methods
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

CFLAGS     += -DPCMCIAENT -DPM_SUPPORT

VPATH		= ../common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS	= cfgpcent

OFILES          = cfgpcent.o cfgcommon.o cfgtoolsx.o chkslot.o coment.o \
		cfg_runtime_br_rspc.o  \
		cfg_cdli.o cfg_get_dvdr.o

INCFLAGS = -I${MAKETOP}/rspc/kernext/pcmcia \
           -I${MAKETOP}/bos/usr/lib/methods/common \
	   -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

ILIST		= cfgpcent
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

IMPORTS         = ${MAKETOP}/rspc/kernext/pcmcia/cs/pcmciacs.exp
 
.include <${RULES_MK}>
