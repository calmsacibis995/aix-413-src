# @(#)08  1.1  src/bos/usr/sbin/auditconv/Makefile, cmdsaud, bos411, 9428A410j 12/15/93 09:13:22
#
# COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
#
# FUNCTIONS:
#
# ORIGINS: 83
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#

VPATH		= ../audit

PROGRAMS	= auditconv
MSGHDRS		= auditcv_msg.h
CATFILES	= auditcv.cat
HFILES		= ${MSGHDRS}

ILIST		= auditconv
IDIR		= /usr/sbin/

INCFLAGS	= -I../audit

OFILES		= auditconv.o audutil.o

LIBS		= -ls

.include <${RULES_MK}>
