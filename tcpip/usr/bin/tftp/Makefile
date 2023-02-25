# @(#)59	1.1  src/tcpip/usr/bin/tftp/Makefile, tcpfilexfr, tcpip411, GOLD410 12/8/92 11:50:03
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
#

PROGRAMS                = tftp
MSGHDRS                 = tftp_msg.h
CATFILES                = tftp.cat

IDIR                    = /usr/bin/
ILIST                   = tftp

#IDIR                    = /usr/lib/boot/
#ILIST                   = tftp

CCTYPE                  = writable_strings
CFLAGS                  = -DAIX -DS5 -D_BSD=44 -DINET  -DSDB -DMANYGROUPS
HFILES                  = ${MSGHDRS}
OFILES                  = main.o tftp.o aix_tftp.o tftpsubs.o
LIBS 			= -l bsd

.include <${RULES_MK}>
