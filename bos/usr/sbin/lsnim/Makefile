# @(#)43        1.6  src/bos/usr/sbin/lsnim/Makefile, cmdnim, bos41J, 9517B_all 4/28/95 14:20:32
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/sbin/lsnim/Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= lsnim

OPT_LEVEL	= -g
CFLAGS		= -D_BSD -q dbxextra 
LDFLAGS		+= -s

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

lsnim_OFILES 		= lsnim.o

LIBS		= -lodm -lnim -lmstr

CLEANFILES	= lsnim lsnim.o

.include <${RULES_MK}>


