# @(#)51        1.1  src/bos/usr/sbin/pse/utils/mtest/Makefile, Makefiles, bos411, 9428A410j 8/5/92 18:00:31
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

PROGRAMS	= mtest
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

CFLAGS		= -DFORKER -DNULLTEST -DIPSOCK -DXNSTLI

OFILES		= ipsock.o mtest.o iptli.o nulltest.o xnstli.o osmtest.o

mtest_IMPORTS	= -bI:pse.exp

LIBS		= -ltli -lmi

.include <${RULES_MK}>
