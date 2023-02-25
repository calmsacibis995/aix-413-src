# @(#)40        1.9  src/bos/usr/sbin/trace/Makefile, cmdtrace, bos41B, 412_41B_sync 11/29/94 10:18:17
# COMPONENT_NAME: CMDTRACE
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# @IBM_COPYRIGHT@
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
#  LEVEL 1, 5 Years Bull Confidential Information
#  

VPATH           += :${TARGET_MACHINE}
SUBDIRS         = ${${TARGET_MACHINE}_SUBDIRS}

PROGRAMS	= trace trcload
SCRIPTS		= trace_smutil
MSGHDRS		= cmdtrace_msg.h
CATFILES	= cmdtrace.cat
HFILES		= ${MSGHDRS}

ILIST		= trace ${SCRIPTS} trclist trcload
IDIR		= /usr/lib/ras/
trace_IDIR	= /usr/sbin/
trcload_IDIR    = /usr/lib/

trace_OFILES	= trcdead.o main.o data.o lgen.o trc_test.o child.o  \
		  read_nano.o read_tic.o lock_sema.o
trcload_OFILES  = trc_load.o mk_sp_file.o lock_sema.o data.o
trace_LIBS	= -lras 
trcload_LIBS	= -lras -lodm -lcfg 

.include <${RULES_MK}>
