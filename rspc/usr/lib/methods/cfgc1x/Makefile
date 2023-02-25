# @(#)64    1.4  src/rspc/usr/lib/methods/cfgc1x/Makefile, isax25, rspc411, 9430C411a  7/26/94  12:35:33
#
# COMPONENT_NAME: (ISAX25)
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}bos/usr/lib/methods/common
VPATH		:=${VPATH}:${MAKETOP}bos/usr/lib/methods/cfgcommon

PROGRAMS	= cfgc1x

OFILES		=	cfgcommon.o \
			cfg_no_dvdr.o \
			cfg_runtime_br_rspc.o \
			cfgtools.o \
			chkslot.o \
			ddstools.o \
			cfgc1x.o \
			lducode.o \
			ldutils.o

INCFLAGS	= -I${MAKETOP}bos/usr/lib/methods/common \
		  -I${MAKETOP}bos/usr/lib/methods/cfgcommon

ILIST		= cfgc1x
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
