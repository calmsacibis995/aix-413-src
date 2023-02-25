# @(#)21	1.1  src/tcpip/usr/sbin/rmnamsv/Makefile, tcpnaming, tcpip411, GOLD410 12/8/92 11:43:06
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

PROGRAMS                = rmnamsv
MSGHDRS                 = rmnamsv_msg.h
CATFILES                = rmnamsv.cat

IDIR                    = /usr/sbin/
ILIST                   = rmnamsv

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
HFILES                  = ${MSGHDRS}
LIBS                    = -lffdb -lsmit -lbsd -lcfg -lodm

.include <${RULES_MK}>
