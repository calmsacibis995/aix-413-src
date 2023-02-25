# @(#)49        1.7  src/bos/usr/sbin/nimclient/Makefile, cmdnim, bos41J, 9517B_all 4/28/95 14:22:09
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/sbin/nimclient/Makefile
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

PROGRAMS	= nimclient
OPT_LEVEL	=

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

CFLAGS		= -D_BSD -g -qdbxextra
LDFLAGS		+= -s

nimclient_OFILES 		=  do_nim.o set_date.o nimclient.o

LIBS		= -lodm -lnim -lmstr

.include <${RULES_MK}>
