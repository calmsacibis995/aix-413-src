# @(#)00	1.3  src/tcpip/usr/sbin/inetd/Makefile, tcpinet, tcpip411, 9433B411a 8/17/94 09:36:24
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# (sub)Makefile for inetd
#

PROGRAMS                = inetd
MSGHDRS                 = inetd_msg.h
CATFILES                = inetd.cat

ILIST                   = inetd
IDIR                    = /usr/sbin/

CCTYPE                  = writable_strings
CFLAGS                  = -D_BSD -D_SUN
HFILES                  = ${MSGHDRS}
LIBS                    = -lbsd -lbsdutil -lsrc

.include <${RULES_MK}>
