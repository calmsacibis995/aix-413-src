# @(#)11        1.5  src/bos/usr/sbin/install/sysck/Makefile, cmdinstl, bos411, 9428A410j 1/15/94 17:45:36
# COMPONENT_NAME: cmdinstl
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= sysck
MSGHDRS		= sysck_msg.h
CATFILES	= sysck.cat
HFILES		= ${MSGHDRS}

ILIST		= sysck
IDIR		= /usr/bin/

OFILES		= comhash.o sysck.o sysacl.o sysadd.o \
		  sysaudit.o sysdbio.o sysdel.o sysentry.o \
		  sysinst.o syslink.o sysmode.o syssrc.o \
		  sysutil.o 

LIBS		= -ls -lswvpd -lodm -linulib

.include <${RULES_MK}>
