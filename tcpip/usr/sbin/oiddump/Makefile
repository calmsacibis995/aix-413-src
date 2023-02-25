# @(#)89	1.1  src/tcpip/usr/sbin/oiddump/Makefile, snmp, tcpip411, GOLD410 3/5/93 11:43:50
#
# COMPONENT_NAME: (SNMP) Simple Network Management Protocol Information Utility
#
# ORIGINS: 27
#
# (C) Copyright International Business Machines Corp. 1991, 1993
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#DEFINES		 = -DLOCAL
LOCALCFLAGS              = ${DEFINES}

.include <${MAKETOP}tcpip/usr/Make.cfg>

PROGRAMS                = oiddump

IDIR                    = /usr/sbin/
ILIST                   = oiddump

CCTYPE                  = writable_strings
OFILES                  = oiddump.o

LIBS                    = -lsnmp -lisode

.include <${RULES_MK}>
