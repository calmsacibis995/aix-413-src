# @(#)28        1.2  src/bos/usr/sbin/lvm/liblv/Makefile, cmdlvm, bos411, 9428A410j 9/13/92 14:56:26
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

VPATH			= ..

SHARED_LIBRARIES	= libsm.a
EXPLIB_TARGETS		= export_libsm.a

MSGHDRS			= cmdlvm_msg.h

SHARED_OFILES		= util.o util3.o util8.o lvodm_lib.o lsutil.o \
			  lvcb.o lprnterr.o lvm_msg.o

NOSHARED_OFILES		= set_flg.o

EXPORTS			= -bE:libsm.exp

LIBS			= libc.a libodm.a libcfg.a liblvm.a

INCFLAGS		= -I../include -I..

ILIST			= libsm.a
IDIR			= /usr/lib/

.include <${RULES_MK}>

util8.o	lvodm_lib.o lsutil.o lprnterr.o lvm_msg.o:	cmdlvm_msg.h
lvm_msg.o:	cmdlvm_def.h
