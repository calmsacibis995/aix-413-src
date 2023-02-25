# @(#)56        1.1  src/bos/usr/sbin/strerr/Makefile, cmdpse, bos411, 9428A410j 8/5/92 18:01:40
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

VPATH		= ${MAKETOP}/bos/kernext/exp

PROGRAMS	= strerr
MSGHDRS		= strerr_msg.h
CATFILES	= strerr.cat
HFILES		= ${MSGHDRS}

ILIST		= strerr
IDIR		= /usr/sbin/

strerr_IMPORTS	= -bI:pse.exp

LIBS		= -lmi

.include <${RULES_MK}>
