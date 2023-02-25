# @(#)05        1.5  src/rspc/usr/lib/methods/cfgkm_isa/Makefile, rspccfg, rspc41J, 9519A_all 5/9/95 08:24:33
# COMPONENT_NAME: rspccfg
#
# FUNCTIONS: N/A
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

VPATH       = ${MAKETOP}/bos/usr/lib/methods/common:${MAKETOP}/bos/usr/lib/methods/cfgktsm
VPATH       := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS    = cfgkm_isa

OFILES      = cfgcommon.o cfgtools.o cfgktsm.o cfg_runtime_br_rspc.o \
	      cfg_dvdr.o cfg_get_dvdr.o util.o ddstools.o

INCFLAGS    = -I${MAKETOP}/bos/usr/lib/methods/common \
	      -I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
	      -I${MAKETOP}/bos/usr/lib/methods/cfgktsm

ILIST       = cfgkm_isa
IDIR        = /usr/lib/methods/


LIBS        = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
