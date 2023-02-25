# @(#)47        1.9  src/bos/usr/sbin/nimesis/Makefile, cmdnim, bos41J, 9517B_all  4/28/95  14:23:29
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/sbin/nimesis/Makefile
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

PROGRAMS	= nimesis 
OPT_LEVEL	=

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

CFLAGS		= -D_BSD -g -qdbxextra
LDFLAGS         += -s

nimesis_OFILES 		= nimesis.o nimkid.o nimreg.o 

LIBS		= -lodm -lnim -lmstr

.include <${RULES_MK}>
