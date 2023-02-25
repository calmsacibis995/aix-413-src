# @(#)01	1.6  src/tcpip/usr/sbin/named/Makefile, tcpnaming, tcpip41J, 9516A_all 4/17/95 14:25:11
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

PROGRAMS                = named named-xfer
SCRIPTS			= newvers
MSGHDRS                 = named_msg.h
CATFILES                = named.cat

IDIR                    = /usr/sbin/
ILIST                   = named named-xfer

CCTYPE                  = writable_strings
CFLAGS                  = -DDEBUG -DSTATS -D_BSD=44

HFILES                  = ${MSGHDRS}

named_OFILES            = db_dump.o db_load.o db_lookup.o db_reload.o \
                        db_save.o db_update.o ns_forw.o ns_init.o \
                        ns_main.o ns_maint.o ns_resp.o ns_req.o \
                        ns_sort.o ns_stats.o db_glue.o version.o
named-xfer_OFILES       = named-xfer.o db_glue.o

LIBS                    = -lbsd -lsrc

.include <${RULES_MK}>

version.c: Version.c newvers.sh
	${RM} -f ${.TARGET} version
	./newvers ${Version.c:P}|| { ${RM} -f ${.TARGET}; ${FALSE}; }
