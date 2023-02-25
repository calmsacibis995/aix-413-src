# @(#)29	1.4  src/tcpip/usr/sbin/snmpinfo/Makefile, tcpip, tcpip411, GOLD410 4/20/94 14:42:50
#
# COMPONENT_NAME: (SNMP) Simple Network Management Protocol Information Utility
#
# ORIGINS: 27
#
# (C) Copyright International Business Machines Corp. 1991, 1994
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# to build an snmpinfo that accepts traps, uncomment DEFINES and replace
# the uncommented NEWOBJS and comment out the old one.  THe snmpinfo will
# run like snmpinfo, unless named trapmon.
#
# DEFINES		 = -DLOCAL
# NEWOBJS		 = processt.o stats.o
NEWOBJS			 =
LOCALCFLAGS              = ${DEFINES}

.include <${MAKETOP}tcpip/usr/Make.cfg>

PROGRAMS                = snmpinfo

IDIR                    = /usr/sbin/
ILIST                   = snmpinfo

CCTYPE                  = writable_strings
OFILES                  = snmpinfo.o process.o ${NEWOBJS}

LIBS                    = -lsnmp -lisode

.include <${RULES_MK}>
