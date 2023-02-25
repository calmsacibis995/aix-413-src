# @(#)12        1.4  src/bos/usr/sbin/install/bffcreate/Makefile, cmdinstl, bos411, 9428A410j 1/19/94 18:13:32
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


PROGRAMS	= bffcreate
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= bffcreate
IDIR		= /usr/sbin/

INCFLAGS	= -I../messages -I../include

OFILES		= bffcreate.o bff.o inu_bff_eval_size.o sort_prod_list.o

LIBS		= -linulib -lodm -lcfg

.include <${RULES_MK}>
