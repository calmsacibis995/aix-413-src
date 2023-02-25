# @(#)85	1.2  src/rspc/usr/lib/methods/cfgpcasync/Makefile, pcmciafax, rspc41J, 9512A_all 3/20/95 22:13:02
# COMPONENT_NAME: pcmciafax
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

CFLAGS		+= -DPCMCIA -DPM_SUPPORT

PROGRAMS	= cfgpcasync

OFILES		= cfgpcasync.o cfgcommon.o cfg_runtime_br_rspc.o \
		  cfg_dvdr.o cfg_get_dvdr.o cfgtools.o

INCFLAGS        += -I${MAKETOP}/bos/usr/lib/methods/common \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
		   -I${MAKETOP}rspc/kernext/pcmcia/tty \
		   -I${MAKETOP}rspc/kernext/isa/tty

IMPORTS		= ${MAKETOP}rspc/kernext/pcmcia/cs/pcmciacs.exp

ILIST		= cfgpcasync
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
