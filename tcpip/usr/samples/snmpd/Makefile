# @(#)46	1.4  src/tcpip/usr/samples/snmpd/Makefile, tcpip, tcpip411, GOLD410 1/21/94 17:45:17
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
.include <${MAKETOP}tcpip/usr/Make.cfg>

VPATH			= ${TARGET_MACHINE}
SUBDIRS			= dpi smux
DATAFILES		= mibII.my smi.my unix.my view.my ibm.my \
			  ethernet.my token_ring.my generic.my fddi.my

ILIST			= ${DATAFILES}
IDIR			= /usr/samples/snmpd/
IMODE			= 644

.include <${RULES_MK}>
