# @(#)22        1.2  src/bos/usr/sbin/install/inurest/Makefile, cmdinstl, bos411, 9428A410j 11/6/92 12:23:50
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

PROGRAMS	= inurest
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= inurest
IDIR		= /usr/sbin/

INCFLAGS	=-I../messages -I../include

OFILES		= inurest.o inurest_eval_size.o

LIBS		= -linulib -ls -lcfg -lodm

.include <${RULES_MK}>
