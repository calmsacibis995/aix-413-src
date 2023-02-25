# @(#)23        1.5  src/rspc/usr/lib/methods/cfgataide/Makefile, rspccfg, rspc41J, 9516A_all 4/18/95 07:35:10
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

VPATH		= ${MAKETOP}bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}bos/usr/lib/methods/cfgcommon

PROGRAMS	= cfgataide

OFILES          =       cfgcommon.o     \
                        cfg_runtime_br_rspc.o \
                        cfg_dvdr.o \
                        cfg_get_dvdr.o \
                        cfgtoolsx.o     \
                        pvidtools.o     \
                        cfgataide.o     \
                        cfgidedc.o      \
                        idecheck.o


INCFLAGS	= -I${MAKETOP}bos/usr/lib/methods/common	\
		  -I${MAKETOP}bos/usr/lib/methods/cfgcommon	\
		  -I${MAKETOP}rspc/kernext/isa/ide


CFLAGS		= -DPM_SUPPORT

ILIST		= cfgataide
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
