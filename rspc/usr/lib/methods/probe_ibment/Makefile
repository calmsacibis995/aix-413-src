# @(#)67	1.2  src/rspc/usr/lib/methods/probe_ibment/Makefile, isaent, rspc411, 9435D411a 9/2/94 12:55:54
#
#   COMPONENT_NAME: isaent
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ../common

PROGRAMS	= probe_ibment

OFILES		= probe_ibment.o loc_rspc.o

INCFLAGS        = -I${MAKETOP}bos/kernel/sys/${TARGET_MACHINE} \
		  -I${MAKETOP}bos/usr/include \
		  -I${MAKETOP}bos/usr/lib/methods/common \
		  -I../common 

CFLAGS		= -q cpluscmt

ILIST		= probe_ibment
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
