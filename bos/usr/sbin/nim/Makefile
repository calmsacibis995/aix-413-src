# @(#)44        1.3  src/bos/usr/sbin/nim/Makefile, cmdnim, bos41J, 9517B_all 4/28/95 14:20:51
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/sbin/nim/Makefile
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

PROGRAMS	= nim

OPT_LEVEL	= -g
CFLAGS		= -D_BSD -q dbxextra 
LDFLAGS		+= -s

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

nim_OFILES 		= nim.o

LIBS		= -lodm -lnim -lmstr

CLEANFILES	= nim nim.o

.include <${RULES_MK}>


