# @(#)64  1.1  src/rspc/usr/sbin/pcmciastat/Makefile, pcmciaker, rspc411, 9433A411a 8/11/94 20:48:35
#
#   COMPONENT_NAME: PCMCIAKER
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#

#CFLAGS     = -DCFGDEBUG
INCFLAGS  += -I${MAKETOP}/bos/usr/lib/methods/common \
	     -I${MAKETOP}/bos/usr/sbin/mkdev

PROGRAMS   = pcmciastat
MSGHDRS    = pcmciastat_msg.h
CATFILES   = pcmciastat.cat
HFILES     = ${MSGHDRS}

OFILES     = pcmciastat.o get_ut.o
IMPORTS    = $(MAKETOP)rspc/kernext/pcmcia/cs/pcmciacs.exp

LIBS       = -ls -lodm -lcfg -lrts

ILIST       = pcmciastat
IDIR       = /usr/sbin/

.include <${RULES_MK}>

