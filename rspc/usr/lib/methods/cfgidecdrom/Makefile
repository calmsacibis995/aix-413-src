# @(#)34        1.3  src/rspc/usr/lib/methods/cfgidecdrom/Makefile, rspccfg, rspc41J, 9510A 2/21/95 00:49:52
#
# COMPONENT_NAME: (rspccfg)
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1,  5 Years Bull Confidential Information

VPATH		= ${MAKETOP}bos/usr/lib/methods/common

PROGRAMS	= cfgidecdrom

OFILES		= cfgdevice.o cfgtools.o cfgidecdrom.o chkslot.o 

INCFLAGS	= -I${MAKETOP}bos/usr/lib/methods/common \
                  -I${MAKETOP}rspc/kernext/idecdrom

CFLAGS		= -DPM_SUPPORT

ILIST		= cfgidecdrom
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
