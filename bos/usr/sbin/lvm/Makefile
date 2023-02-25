# @(#)07        1.1  src/bos/usr/sbin/lvm/Makefile, cmdlvm, bos411, 9428A410j 8/31/92 19:05:55
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

EXPLIB_SUBDIRS	= liblv

SUBDIRS		= liblv intercmd highcmd

PROGRAMS	= lchangelv lchangepv lcreatelv lcreatevg ldeletelv \
		  ldeletepv lextendlv linstallpv lmigratepp lquerylv \
		  lquerypv lqueryvg lqueryvgs lreducelv lresynclp \
		  lresynclv lresyncpv lvaryoffvg lvaryonvg
MSGHDRS		= cmdlvm_msg.h
CATFILES	= cmdlvm.cat
HFILES		= ${MSGHDRS} cmdlvm_def.h

INCLUDES	= cmdlvm_msg.h
EXPDIR		= /usr/include/
ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

LIBFLAGS	= -Lliblv

LIBS		= -lsm -lodm -lcfg -llvm -ls

.include <${RULES_MK}>
