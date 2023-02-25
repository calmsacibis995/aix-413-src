# @(#)25        1.4  src/bos/usr/sbin/lsgroup/Makefile, cmdsuser, bos411, 9428A410j 10/7/93 12:43:58
#
#   COMPONENT_NAME: CMDSUSER
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/bin/newgrp

PROGRAMS	= lsgroup
MSGHDRS		= tcbauth_msg.h
CATFILES	= tcbauth.cat
HFILES		= ${MSGHDRS}

ILIST		= lsgroup
IDIR		= /usr/sbin/

INCFLAGS	= -I${MAKETOP}/bos/usr/bin/newgrp

OFILES		= lsgroup.o util.o chk_util.o ls_util.o lsgratab.o

LIBS		= -ls

.include <${RULES_MK}>
