# @(#)13        1.10  src/bos/usr/sbin/install/ckprereq/Makefile, cmdinstl, bos411, 9428A410j 6/23/94 11:43:37

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

VPATH		= ../messages

PROGRAMS	= ckprereq
LIBRARIES	= libckprereq.a
EXPLIB_TARGETS	= export_libckprereq.a
CATFILES	= 
HFILES		= 

ILIST		= ckprereq
IDIR		= /usr/sbin/

INCFLAGS	= -I../include -I../messages


libckprereq.a_IDIR	= /usr/ccs/lib/

libckprereq.a_OFILES	= \
                  build_graph.o \
                  build_supersede.o \
		  copy_utils.o \
                  cycle_utils.o \
                  determine_order.o \
                  evaluate_base_levels.o \
                  evaluate_failures.o \
                  evaluate_graph.o \
                  evaluate_requisite.o \
                  evaluate_supersedes.o \
                  fix_state_utils.o \
		  graph_dump.o \
                  hash_utils.o \
                  inu_check_prereq.o \
                  list_utils.o \
                  load_fix_info.o \
                  mem_utils.o \
                  my_lex.o \
                  parser.o \
                  process_unres_updts.o \
                  report_failures.o \
                  report_successes.o \
                  report_utils.o \
                  report_warnings.o \
		  utils.o 

LIBS		= -lckprereq -linulib -lswvpd -lodm

.include <${RULES_MK}>
