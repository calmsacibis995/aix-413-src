# @(#)86        1.3  src/bos/usr/sbin/cron/Makefile, cmdcntl, bos411, 9428A410j 10/19/93 17:06:49
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

PROGRAMS	= at crontab cron atq atrm cronadm
SCRIPTS		= batch .proto at.deny cron.deny log
MSGHDRS		= cron_msg.h
CATFILES	= cron.cat
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS} ${SCRIPTS} adm root sys queuedefs
IDIR		= /usr/bin/

.proto_IDIR	= /var/adm/cron/
at.deny_IDIR	= /var/adm/cron/
cron.deny_IDIR	= /var/adm/cron/
log_IDIR	= /var/adm/cron/
queuedefs_IDIR	= /var/adm/cron/
adm_IDIR	= /var/spool/cron/crontabs/
root_IDIR	= /var/spool/cron/crontabs/
sys_IDIR	= /var/spool/cron/crontabs/
cron_IDIR	= /usr/sbin/

cronadm_OFILES	= cronsub.o elm.o permit.o funcs.o cronadm.o
crontab_OFILES	= crontab.o permit.o funcs.o cronsub.o
at_OFILES	= at.o att1.o permit.o funcs.o cronsub.o
cron_OFILES	= cron.o funcs.o elm.o
atq_OFILES	= atq.o cronsub.o funcs.o
atrm_OFILES	= atrm.o cronsub.o funcs.o

LIBS		= -ls

.include <${RULES_MK}>

att1.c : att1.h
