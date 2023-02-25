# @(#)70	1.3  src/rspc/usr/sbin/acfgd/Makefile, pcmciaker, rspc41J, 9512A_all 3/16/95 17:11:27
#
#   COMPONENT_NAME: PCMCIAKER
#
#   FUNCTIONS: Makefile for auto-config daemon
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

CFLAGS     += -DPMLIB_ENABLE
#CFLAGS     += -DCFGDEBUG -DPMLIB_ENABLE
#CFLAGS     += -DCFGDEBUG
INCFLAGS   += -I${MAKETOP}/bos/usr/lib/methods/common \
              -I${MAKETOP}/rspc/kernext/pcmcia/cs

PROGRAMS   = acfgd
SCRIPTS    = lacfgd

OFILES     = acfgd.o detector.o rdevice.o
IMPORTS    = $(MAKETOP)rspc/kernext/pcmcia/cs/evh.exp

#LIBS       = -ls -lodm -lcfg -lrts -lc
LIBS       = -lpm -ls -lodm -lcfg -lrts -lc

ILIST       = ${PROGRAMS} ${SCRIPTS}
IDIR       = /usr/sbin/

.include <${RULES_MK}>
