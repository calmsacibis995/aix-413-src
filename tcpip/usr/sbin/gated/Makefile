# @(#)99	1.3  src/tcpip/usr/sbin/gated/Makefile, tcprouting, tcpip411, GOLD410 12/7/93 13:25:14
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH=${MAKETOP}bos/kernel/sys:.

LOCALCFLAGS              = -DBSD4_4 -DIBM_LIBSMUX

.include <${MAKETOP}tcpip/usr/Make.cfg>

PROGRAMS                = ripquery ospf_monitor gdc gated
MSGHDRS                 = gated_msg.h
CATFILES                = gated.cat

IDIR                    = /usr/sbin/
ILIST                   = gated

CCTYPE                  = writable_strings

ripquery_CFILES=	ripquery.c \
			inet_aton.c \
			str.c \
			standalone.c

ospf_monitor_CFILES=	ospf_monitor.c \
			inet_aton.c \
			str.c \
			standalone.c \
			checksum.c

gdc_CFILES=		gdc.c \
			inet_aton.c \
			str.c \
			standalone.c

gated_CFILES=	checksum.c \
	if.c \
	if_rt.c \
	inet.c \
	inet_aton.c \
	krt.c \
	krt_rtread_kinfo.c \
	krt_ifread_ioctl.c \
	krt_rt_sock.c \
	krt_lladdr_kmem.c \
	krt_symbols_nlist.c \
	krt_ipmulticast.c \
	kvm.c \
	main.c \
	policy.c \
	rt_table.c \
	rt_static.c \
	rt_radix.c \
	rt_redirect.c \
	rt_mib.c \
	scram.c \
	sockaddr.c \
	str.c \
	targets.c \
	task.c \
	trace.c \
	parse.c \
	aspath.c \
	bgp.c \
	bgp_init.c \
	bgp_rt.c \
	bgp_mib.c \
	egp.c \
	egp_init.c \
	egp_rt.c \
	egp_mib.c \
	hello.c \
	icmp.c \
	ospf_build_dbsum.c \
	ospf_build_ls.c \
	ospf_choose_dr.c \
	ospf_conf.c \
	ospf_flood.c \
	ospf_init.c \
	ospf_log.c \
	ospf_lsdb.c \
	ospf_mib.c \
	ospf_newq.c \
	ospf_rt.c \
	ospf_rtab.c \
	ospf_rxlinkup.c \
	ospf_rxmon.c \
	ospf_rxpkt.c \
	ospf_spf.c \
	ospf_spf_leaves.c \
	ospf_states.c \
	ospf_tqhandle.c \
	ospf_trace.c \
	ospf_txpkt.c \
	rip.c \
	snmp_isode.c

ripquery_OFILES=	ripquery.o \
			inet_aton.o \
			str.o \
			standalone.o

ospf_monitor_OFILES=	ospf_monitor.o \
			inet_aton.o \
			str.o \
			standalone.o \
			checksum.o

gdc_OFILES=		gdc.o \
			inet_aton.o \
			str.o \
			standalone.o

gated_OFILES=	checksum.o \
	if.o \
	if_rt.o \
	inet.o \
	inet_aton.o \
	krt.o \
	krt_rtread_kinfo.o \
	krt_ifread_ioctl.o \
	krt_rt_sock.o \
	krt_lladdr_kmem.o \
	krt_symbols_nlist.o \
	krt_ipmulticast.o \
	kvm.o \
	main.o \
	policy.o \
	rt_table.o \
	rt_static.o \
	rt_radix.o \
	rt_redirect.o \
	rt_mib.o \
	scram.o \
	sockaddr.o \
	str.o \
	targets.o \
	task.o \
	trace.o \
	parser.o \
	parse.o \
	lexer.o \
	version.o \
	aspath.o \
	bgp.o \
	bgp_init.o \
	bgp_rt.o \
	bgp_mib.o \
	egp.o \
	egp_init.o \
	egp_rt.o \
	egp_mib.o \
	hello.o \
	icmp.o \
	ospf_build_dbsum.o \
	ospf_build_ls.o \
	ospf_choose_dr.o \
	ospf_conf.o \
	ospf_flood.o \
	ospf_init.o \
	ospf_log.o \
	ospf_lsdb.o \
	ospf_mib.o \
	ospf_newq.o \
	ospf_rt.o \
	ospf_rtab.o \
	ospf_rxlinkup.o \
	ospf_rxmon.o \
	ospf_rxpkt.o \
	ospf_spf.o \
	ospf_spf_leaves.o \
	ospf_states.o \
	ospf_tqhandle.o \
	ospf_trace.o \
	ospf_txpkt.o \
	rip.o \
	snmp_isode.o \
	gated-mib.o

HFILES=	 ${MSGHDRS} \
	defs.h \
	config.h \
	if.h \
	include.h \
	inet.h \
	krt.h \
	krt_var.h \
	policy.h \
	rt_table.h \
	rt_var.h \
	scram.h \
	sockaddr.h \
	str.h \
	targets.h \
	task.h \
	trace.h \
	parser.h \
	parse.h \
	unix.h \
	task_sig.h \
	paths.h \
	aspath.h \
	bgp.h \
	egp.h \
	egp_param.h \
	hello.h \
	icmp.h \
	ip_icmp.h \
	ospf.h \
	ospf_const.h \
	ospf_gated.h \
	ospf_gated_mem.h \
	ospf_log.h \
	ospf_lsdb.h \
	ospf_pkts.h \
	ospf_rtab.h \
	ospf_timer_calls.h \
	rip.h \
	snmp_isode.h


LIBS                    = -lsnmp -lisode -lm -lsrc -lbsd -lunpack

.include <${RULES_MK}>

task.o: task_sig.h 

task_sig.h: signal.h sigconv.awk
	${AWK} -f ${sigconv.awk:P} ${signal.h:P} > task_sig.h

_CFILES 		= ${CFILES:@TMP@${${TMP}:P}@}

_HFILES 		= ${HFILES:@TMP@${${TMP}:P}@}

#version.c: version.awk ${CFILES} ${HFILES}
#	${GREP} '$$Header' ${_CFILES} ${_HFILES} | ${AWK} -f ${version.awk:P} - > version.c || { ${RM} -f ${.TARGET}; false; }

version.c: 
	@${ECHO} 'const char *gated_version = "3.0.2";' > version.c
	@${ECHO} 'const char *build_date = "'`${DATE}`'";' >> version.c
