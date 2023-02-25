# @(#)78        1.2  src/bos/usr/sbin/syslogd/Makefile, cmdnet, bos411, 9428A410j 3/28/94 11:32:12
# COMPONENT_NAME:
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

PROGRAMS	= syslogd
MSGHDRS		= syslogd_msg.h
CATFILES	= syslogd.cat
HFILES		= ${MSGHDRS}

ILIST		= syslogd
IDIR		= /usr/sbin/

CFLAGS		= -D_BSD=44 -DMAXLOGINS=16

LIBS		= -lsrc -ls -lrts
NOUPDATE_LIST	= \
/etc/syslog.conf


.include <${RULES_MK}>
