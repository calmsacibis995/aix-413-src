# @(#)00        1.2  src/rspc/usr/lib/methods/cfgfda_isa/Makefile, rspccfg, rspc41J, 9513A_all 3/28/95 13:56:50
# COMPONENT_NAME: rspccfg
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

CFLAGS		+= -DISA

VPATH		+= ${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgfda

PROGRAMS	= cfgfda_isa

OFILES		= cfgcommon.o cfg_runtime_br_rspc.o cfg_dvdr.o \
		  cfg_get_dvdr.o cfgtools.o  cfgfda.o

INCFLAGS        += -I${MAKETOP}/bos/usr/lib/methods/common \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgfda


ILIST		= cfgfda_isa
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
