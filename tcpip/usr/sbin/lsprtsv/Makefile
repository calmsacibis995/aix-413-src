# @(#)14	1.1  src/tcpip/usr/sbin/lsprtsv/Makefile, tcpip, tcpip411, GOLD410 12/8/92 11:41:57
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

PROGRAMS                = lsprtsv
MSGHDRS                 = lsprtsv_msg.h
CATFILES                = lsprtsv.cat

IDIR                    = /usr/sbin/
ILIST                   = lsprtsv

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
HFILES                  = ${MSGHDRS}
LIBS                    = -lffdb -lsmit -lbsd -lcfg -lodm

.include <${RULES_MK}>
