# @(#)07	1.4  src/bos/usr/sbin/bootpd/Makefile, cmdnet, bos411, 9428A410j 3/28/94 16:10:05
# 
# COMPONENT_NAME: CMDNET Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS                = bootpd

ILIST                   = bootpd bootptab in_netarp.h bootpd.mak \
			bootp.h bootpd.c bootpd.h hash.c hash.h	\
			version.c readfile.c

bootpd_IDIR		= /usr/sbin/
bootptab_IDIR		= /etc/
IDIR                    = /usr/samples/tcpip/bootpd/

CCTYPE                  = writable_strings

CFLAGS 			= -DPRIVATE=static -DFEATURE -DAIX32 -DAIXBUILD

CFILES                  = bootpd.c hash.c readfile.c version.c

OFILES                  = bootpd.o readfile.o hash.o version.o

HFILES                  = bootp.h bootpd.h hash.h in_netarp.h

#LIBS                    = -lbsd

NOUPDATE_LIST		= /etc/bootptab 


.include <${RULES_MK}>

#version.c: version.awk ${CFILES} ${HFILES}
#	fgrep '$$Header' ${_CFILES} ${_HFILES} | awk -f ${version.awk:P} - > version.c || { rm -f ${.TARGET}; false; }
#	@echo 'const char *build_date = "'`date`'";' >> version.c
#

