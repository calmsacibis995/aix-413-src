# @(#)75	1.6  src/tcpip/usr/lib/libsnmp/Makefile, tcpip, tcpip411, 9434A411a 8/18/94 14:17:31
# COMPONENT_NAME: TCPIP - SNMP library
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}tcpip/usr/Make.cfg>

#LOCALCFLAGS 	= -D_ANSI_C_SOURCE -D_ALL_SOURCE -D_POSIX_SOURCE

LIBRARIES		= libsnmp.a
MSGHDRS			= snmpd_msg.h
INCLUDES		= ${MSGHDRS}
CATFILES		= snmpd.cat
HFILES			= ${MSGHDRS}

EXPLIB_TARGETS		= export_libsnmp.a

ILIST			= libsnmp.a
IDIR			= /usr/lib/
IMODE			= 644

snmpd_msg.h_IDIR	= /usr/include/

PYOFILES		= SMI_tables.o SNMP_tables.o SMUX_tables.o

# PYOFILES must be built first.
OFILES 			= ${PYOFILES} objects.o syntax.o moresyntax.o \
			values.o qb_cpy.o io.o dgram.o smux.o smuxentry.o \
			oid_ocpy.o alarm.o new_message.o sndrcv.o global.o \
			advise.o adios.o lookup_host.o lookup_addr.o \
			dumpbits.o str2int.o dg_flushb.o cpyosyn.o 

.include <${RULES_MK}>

