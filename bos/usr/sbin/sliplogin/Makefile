# @(#)39	1.1  src/bos/usr/sbin/sliplogin/Makefile, cmdnet, bos411, 9428A410j 9/27/93 13:32:44
# COMPONENT_NAME: CMDNET
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

VPATH		= ${MAKETOP}/bos/usr/sbin/sliplogin

PROGRAMS	= sliplogin
MSGHDRS		= sliplogin_msg.h
CATFILES	= sliplogin.cat
DATAFILES	= slip.hosts slip.login slip.logout slipcall
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS} ${DATAFILES}
IDIR		= /usr/sbin/
slip.hosts_IDIR	= /etc/
slip.login_IDIR	= /etc/
slip.logout_IDIR	= /etc/

CFLAGS		= -D_BSD=44 

INCFLAGS	= -I${MAKETOP}/bos/usr/bin/uucp

sliplogin_OFILES		= sliplogin.o

.include <${RULES_MK}>
