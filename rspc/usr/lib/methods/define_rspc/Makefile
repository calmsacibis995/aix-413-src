# @(#)43        1.2  src/rspc/usr/lib/methods/define_rspc/Makefile, rspccfg, rspc411, GOLD410 6/9/94 16:32:22
#
# COMPONENT_NAME:  RSPCCFG
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common

PROGRAMS	= define_rspc

OFILES		= define_rspc.o loc_rspc.o

INCFLAGS        = -I${MAKETOP}bos/usr/include \
		  -I${MAKETOP}bos/usr/lib/methods/common \
		  -I../common

ILIST		= define_rspc
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
