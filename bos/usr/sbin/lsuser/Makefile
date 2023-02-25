# @(#)29        1.4  src/bos/usr/sbin/lsuser/Makefile, cmdsuser, bos411, 9428A410j 10/7/93 12:45:53
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

VPATH		= ${MAKETOP}/bos/usr/bin/newgrp

PROGRAMS	= lsuser
MSGHDRS		= tcbauth_msg.h
CATFILES	= tcbauth.cat
HFILES		= ${MSGHDRS}

ILIST		= lsuser
IDIR		= /usr/sbin/

INCFLAGS	= -I${MAKETOP}/bos/usr/bin/newgrp

OFILES		= lsuser.o util.o chk_util.o ls_util.o lsusatab.o

LIBS		= -ls

.include <${RULES_MK}>
