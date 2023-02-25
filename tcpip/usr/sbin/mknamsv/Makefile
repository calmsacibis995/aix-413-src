# @(#)15	1.1  src/tcpip/usr/sbin/mknamsv/Makefile, tcpnaming, tcpip411, GOLD410 12/8/92 11:42:08
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

PROGRAMS                = mknamsv
MSGHDRS                 = mknamsv_msg.h
CATFILES                = mknamsv.cat

IDIR                    = /usr/sbin/
ILIST                   = mknamsv

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
HFILES                  = ${MSGHDRS}
LIBS                    = -lffdb -lsmit -lbsd -lcfg -lodm

.include <${RULES_MK}>
