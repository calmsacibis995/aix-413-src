# @(#)12       1.1  src/rspc/usr/ccs/lib/libpm/Makefile, pwrcmd, rspc41J, 9510A 3/8/95 07:40:51
#
# COMPONENT_NAME: pwrcmd
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES	= libpm.a
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libpm.a

ILIST		= libpm.a
#BINARIES	= libpm.a
IDIR		= /usr/lib/

#LIBS		=libc.a

CFLAGS		= -U_AIX
#CFLAGS		= -g -U_AIX -PM_DDEBUG

OFILES	= get_event_notice.o \
			register_application.o \
			request_parameter.o \
			pm_sock.o \
			request_battery.o \
			request_state.o
	
.include <${RULES_MK}>
