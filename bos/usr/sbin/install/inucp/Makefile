# @(#)19        1.2  src/bos/usr/sbin/install/inucp/Makefile, cmdinstl, bos411, 9428A410j 11/7/92 13:55:07
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

PROGRAMS	= inucp
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST		= inucp
IDIR		= /usr/sbin/

INCFLAGS	= -I../messages -I../include

OFILES		= inucp.o inucp_eval_size.o

LIBS		= -linulib -ls -lcfg -lodm

.include <${RULES_MK}>
