# @(#)84        1.1  src/bos/usr/sbin/hostid/Makefile, cmdnet, bos411, 9428A410j 8/5/92 17:49:16
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

PROGRAMS	= hostid
MSGHDRS		= hostid_msg.h
CATFILES	= hostid.cat
HFILES		= ${MSGHDRS}

ILIST		= hostid
IDIR		= /usr/sbin/

CFLAGS		= -DS5 -D_BSD=44 -DSLIPDISC=2 -DIP_SECURITY_OPTION -D_CSECURITY

.include <${RULES_MK}>
