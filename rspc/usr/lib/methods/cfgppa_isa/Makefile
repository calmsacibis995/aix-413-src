# @(#)08        1.1  src/rspc/usr/lib/methods/cfgppa_isa/Makefile, cfgmethods, rspc411, GOLD410 6/28/94 08:34:01
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

VPATH		+= ${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgppa

PROGRAMS	= cfgppa_isa

OFILES		= cfgcommon.o cfg_runtime_br_rspc.o cfg_dvdr.o \
		  cfg_get_dvdr.o cfgtools.o  cfgppa.o

INCFLAGS        += -I${MAKETOP}/bos/usr/lib/methods/common \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgppa


ILIST		= cfgppa_isa
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
