# @(#)25	1.3  src/tcpip/usr/sbin/ipreport/Makefile, tcpras, tcpip411, GOLD410 4/8/94 09:49:10
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS                = ipreport
MSGHDRS                 = ipreport_msg.h
CATFILES                = ipreport.cat

IDIR                    = /usr/sbin/
ILIST                   = ipreport

CCTYPE                  = writable_strings
CFLAGS                  = -DLOCAL  -D_BSD=44
HFILES                  = ${MSGHDRS} ipr.h \
			klm_prot.h mount.h nfs.h nlm_prot.h pacl.h \
			prot_lock.h rex.h rstat.h rusers.h rwall.h \
			sm_inter.h sniffer.h spray.h yppasswd.h
OFILES                  = ipreport.o nfs_xdr.o rpc.o misc.o ci.o \
			proc.o nfs_dump.o xdr_nlm.o nlm_dump.o \
			pcnfs_xdrprt.o yp.o pmap.o pacl_xdr.o \
			p-dns.o p-rip.o netware.o checksum.o yppasswd.o
LIBS                    = -lbsd -lsrc

.include <${RULES_MK}>
