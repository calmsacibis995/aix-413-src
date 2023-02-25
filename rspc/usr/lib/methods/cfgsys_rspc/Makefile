# @(#)82        1.2  src/rspc/usr/lib/methods/cfgsys_rspc/Makefile, rspccfg, rspc41J, 9513A_all 3/28/95 16:11:10
#
# COMPONENT_NAME: RSPCCFG
#
# FUNCTIONS:
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

VPATH		+=${MAKETOP}bos/usr/lib/methods/common:${MAKETOP}bos/usr/lib/methods/cfgsys

#CFLAGS		=-DCFGDEBUG			

PROGRAMS	= cfgsys_rspc

OFILES		= cfgsys_rspc.o cfgtools.o setvar.o 

INCFLAGS	= -I${MAKETOP}bos/usr/lib/methods/common


ILIST		= cfgsys_rspc
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
