# @(#)28	1.5  src/tcpip/usr/sbin/snmpd/Makefile, tcpip, tcpip411, GOLD410 1/21/94 17:04:00
#
# COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Licensed Material - Property of IBM
#

LOCALCFLAGS             = -DSMUX -DSRC_SUPPORT -DUNIXGROUP

.include <${MAKETOP}tcpip/usr/Make.cfg>

PROGRAMS                = snmpd

IDIR                    = /usr/sbin/
ILIST                   = snmpd

CCTYPE                  = writable_strings
OFILES                  = ibm.o snmp_g.o mib.o mib_o.o config.o \
			logging.o smux_g.o smuxserv.o src.o view.o \
			traps.o system.o icmp.o udp.o tcp.o vpd.o \
			interfaces.o unix_g.o routes.o ip.o snmpd.o \
			generic.o ethernet.o devices.o token_ring.o \
			timers.o fddi.o

LIBS                    = -lsnmp -lisode -lswvpd -lcfg -lodm -lsrc

.include <${RULES_MK}>
