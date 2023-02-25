# @(#)58        1.4  src/bos/usr/sbin/bootinfo/Makefile, bosboot, bos41J, 9522A_b 5/31/95 11:33:23
# COMPONENT_NAME: BOSBOOT
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


PROGRAMS	= bootinfo
VPATH		= ${MAKETOP}/bos/usr/lib/methods/common
OFILES		= bootinfo.o cfgboot_tools.o
MSGHDRS		= bootinfo_msg.h
CATFILES	= bootinfo.cat
LINTFLAGS	+= ${_CCDEFS_} -wP -Nn3000 -Nd4000

ILIST		= bootinfo
IDIR		= /usr/sbin/

LIBS		= -lodm -lcfg

.include <${RULES_MK}>
