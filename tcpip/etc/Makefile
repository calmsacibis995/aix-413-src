# @(#)98	1.5  src/tcpip/etc/Makefile, tcpip, tcpip41J, 9516A_all 4/17/95 14:42:35
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH			= ${TARGET_MACHINE}
DATAFILES		= 3270.keys 3270keys.hft egpconfig gated.conf \
		        hosts.equiv hosts.lpd inetd.conf map3270 \
		        rc.tcpip securetcpip tcp.clean telnet.conf snmpd.conf \
			snmpd.peers mib.defs isoentities isoaliases \
			isomacros isobjects isoservices 3270_arab_kyb.map

ILIST			= ${DATAFILES}
IDIR			= /etc/
IMODE			= 644

NOUPDATE_LIST		= \
			/etc/snmpd.conf \
			/etc/snmpd.peers \
			/etc/rc.tcpip \
			/etc/hosts.equiv \
			/etc/hosts.lpd \
			/etc/3270.keys \
			/etc/3270keys.hft \
			/etc/map3270 \
			/etc/gated.conf \
			/etc/inetd.conf \
			/etc/telnet.conf 

.include <${RULES_MK}>
