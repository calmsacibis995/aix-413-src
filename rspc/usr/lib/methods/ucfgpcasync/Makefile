# @(#)86	1.1  src/rspc/usr/lib/methods/ucfgpcasync/Makefile, pcmciafax, rspc411, 9437A411a 9/8/94 13:58:29
#
# COMPONENT_NAME: pcmciafax
#
# FUNCTIONS:
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH       += ${MAKETOP}bos/usr/lib/methods/cfgasync:${MAKETOP}bos/usr/lib/methods/cfgtty:${MAKETOP}bos/usr/lib/methods/common

PROGRAMS    = ucfgpcasync

ucfgpcasync_OFILES    = cfgtools.o\
                      commondds.o\
                      lionadapdds.o\
                      rsadapdds.o\
                      ucfgpcasync.o

INCFLAGS    = -I${MAKETOP}bos/usr/lib/methods/common -I${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}

ucfgpcasync_LINTFILES = ucfgpcasync.c

LINTFLAGS   = -wk -u -x

LIBS        = -ls -lodm -lcfg -lrts

ILIST       = ucfgpcasync
IDIR        = /usr/lib/methods/

.include <${RULES_MK}>
