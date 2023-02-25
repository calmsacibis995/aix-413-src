# @(#)41	1.3  src/tcpip/usr/samples/tcpip/Makefile, tcpip, tcpip411, GOLD410 4/20/94 10:03:00
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Licensed Materials - Property of IBM
#

VPATH			= ${TARGET_MACHINE}
SUBDIRS			= onhost
DATAFILES		= addrs.awk anon.ftp gateways hosts.awk named.boot \
			named.data named.hosts netrc networks resolv.conf \
			rhosts tftpaccess.ctl inetbld README

ILIST			= ${DATAFILES}
IDIR			= /usr/samples/tcpip/
IMODE			= 644

.include <${RULES_MK}>
