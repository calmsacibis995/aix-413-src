# @(#)10	1.4  src/tcpip/usr/sbin/tcpdump/Makefile, tcpip, tcpip411, 9434B411a 8/25/94 09:21:21
#
#   COMPONENT_NAME: TCPIP
#
#   FUNCTIONS: none
#
#   ORIGINS: 26,27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= tcpdump

ILIST		= tcpdump
IDIR		= /usr/sbin/

MSGHDRS		= tcpdump_msg.h
CATFILES	= tcpdump.cat
HFILES		= ${MSGHDRS}

CFLAGS		= -D_BSD=44 -DMAXLOGINS=16 -DIP_MULTICAST
LIBS		= -lcfg -lodm

tcpdump_OFILES = version.o tcpdump.o tcpgram.o tcplex.o addrtoname.o \
	print-tr.o print-ether.o print-ip.o print-arp.o print-tcp.o print-udp.o \
	print-atalk.o print-domain.o print-tftp.o print-bootp.o print-nfs.o \
	print-icmp.o nametoaddr.o print-sl.o print-ppp.o print-rip.o \
	print-snmp.o print-ntp.o print-null.o print-egp.o print-ospf.o \
	print-fddi.o print-sunrpc.o print-iso88023.o pcap.o md.o version.o \
	savefile.o util.o etherent.o inet.o gencode.o optimize.o \
	bpf_dump.o bpf_filter.o bpf_image.o bpf_load.o


.include <${RULES_MK}>

tcplex.c: tcplex.l
	${RM} -f tcplex.c
	${LEX} $<
	${MV} lex.yy.c tcplex.c

tcpgram.c: tcpgram.y
	${RM} -f tcpgram.c tokdefs.h
	${YACC} -d $<
	${MV} y.tab.c tcpgram.c
	${MV} y.tab.h tokdefs.h
