# @(#)26	1.1  src/tcpip/usr/sbin/ip_auditpr/Makefile, tcpras, tcpip411, GOLD410 12/8/92 11:43:57
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

PROGRAMS                = ip_auditpr
MSGHDRS                 = ip_auditpr_msg.h
CATFILES                = ip_auditpr.cat

IDIR                    = /usr/sbin/
ILIST                   = ip_auditpr

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD=44
HFILES                  = ${MSGHDRS}
LIBS                    = -lbsd 

.include <${RULES_MK}>
