# @(#)23        1.2  src/bos/usr/sbin/install/inusave/Makefile, cmdinstl, bos411, 9428A410j 11/7/92 15:04:59
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

PROGRAMS	= inusave
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= inusave
IDIR		= /usr/sbin/

INCFLAGS	= -I../messages -I../include

OFILES		= inusave.o inusave_eval_size.o get_member.o save_files.o \
		  save_regfile.o save_arfile.o init_ctr.o strip.o

LIBS		= -linulib -ls -lcfg -lswvpd -lodm

.include <${RULES_MK}>
