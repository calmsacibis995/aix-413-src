# @(#)71	1.7  src/bos/usr/sbin/netstat/Makefile, cmdnet, bos411, 9428A410j 3/29/94 19:42:24
#
#   COMPONENT_NAME: CMDNET
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ${TARGET_MACHINE}

PROGRAMS	= netstat netstat_load
MSGHDRS		= netstat_msg.h
CATFILES	= netstat.cat
HFILES		= ${MSGHDRS}

ILIST		= netstat netstat_load
IDIR		= /usr/sbin/

# INCFLAGS forces the NFS include files to be taken from the source
# tree instead of the export tree since BOS is built before NFS.
# Verify by looking in depend.mk after the build completes.
INCFLAGS	= -I${MAKETOP}/nfs/usr/include
CFLAGS		= -DS5 -DINET -D_BSD=44 -DSDB -DMANYGROUPS -DIP_MULTICAST -DLOCK_NETSTATS -DNETSYNC_LOCK

netstat_ENTRYPOINT	= __start
netstat_IMPORTS	= -bI:packets_load1.imp packets_load2.imp
netstat_EXPORTS	= -bE:packets_load1.imp packets_load2.imp
netstat_LIBS	= -lunpack -lbsd 

netstat_OFILES	= host.o inet.o iso.o ns.o if.o main.o mbuf.o route.o \
		  unix.o drvstats.o packets.o

netstat_load_OFILES	= netstat_load.o
netstat_load_ENTRYPOINT	= netstat_load
netstat_load_IMPORTS	= -bI:netstat_load1.imp netstat_load2.imp
netstat_load_EXPORTS	= -bE:netstat_load1.imp netstat_load2.imp

.include <${RULES_MK}>
