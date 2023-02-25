# @(#)29        1.3  src/rspc/usr/lib/methods/cfgidedisk/Makefile, rspccfg, rspc41J 4/19/95 17:57:39
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

PROGRAMS	= cfgidedisk

OFILES		= cfgdisk.o cfgidedisk.o pvidtools.o cfgtoolsx.o finddisk.o

INCFLAGS	= -I${MAKETOP}bos/usr/lib/methods/common            \
                  -I${MAKETOP}rspc/kernext/idedisk		    \
                  -I${MAKETOP}bos/kernel/sys/POWER

CFLAGS		= -DPM_SUPPORT 

ILIST		= cfgidedisk
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
