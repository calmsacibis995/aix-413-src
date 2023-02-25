# @(#)62        1.4  src/rspc/usr/lib/methods/cfgisatok/Makefile, isatok, rspc41J, 9522A_b 5/31/95 11:38:07
# COMPONENT_NAME: SYSXITOK
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

VPATH		= ../common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS        = cfgisatok

OFILES          = cfgisatok.o \
		cfgcommon.o \
		cfgtoolsx.o \
		cfgboot_tools.o \
		cfg_runtime_br_rspc.o  \
		cfg_cdli.o	\
		cfg_get_dvdr.o

INCFLAGS = -I${MAKETOP}/rspc/kernext/isa -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS := ${INCFLAGS} -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

ILIST           = cfgisatok
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
