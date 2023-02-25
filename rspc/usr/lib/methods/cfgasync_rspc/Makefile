# @(#)18	1.2  src/rspc/usr/lib/methods/cfgasync_rspc/Makefile, rspccfg, rspc41J, 9512A_all 3/20/95 21:58:35
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

VPATH		+= ${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS	= cfgasync_rspc

OFILES		= cfgasync_rspc.o cfgcommon.o cfg_runtime_br_rspc.o \
		  cfg_dvdr.o cfg_get_dvdr.o cfgtools.o

CFLAGS		+= -DPM_SUPPORT

INCFLAGS        += -I${MAKETOP}/bos/usr/lib/methods/common \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
		   -I${MAKETOP}rspc/kernext/isa/tty


ILIST		= cfgasync_rspc
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
