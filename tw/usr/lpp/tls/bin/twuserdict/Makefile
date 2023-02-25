# @(#)32	1.1  src/tw/usr/lpp/tls/bin/twuserdict/Makefile, tw, tw411, GOLD410 12/8/92 15:58:17
#
# COMPONENT_NAME: TW
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= twuserdict
HFILES		= udeph.h udepherr.h

ILIST		= twuserdict
IDIR		= /usr/lpp/tls/bin/

OFILES		= udemain.o udeph.o
LIBS		= -lXm -lXt -lX11 -lIM -lPW -lm

.include <${RULES_MK}>

udemain.o: udemain.h udemainMsg.h
