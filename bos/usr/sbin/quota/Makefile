# @(#)20	1.2  src/bos/usr/sbin/quota/Makefile, cmdquota, bos411, 9428A410j 4/21/93 11:14:38
#
# COMPONENT_NAME: CMDQUOTA
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= quota quotaon quot repquota edquota quotacheck
MSGHDRS		= quota_msg.h quotaon_msg.h quot_msg.h repquota_msg.h \
		  edquota_msg.h quotacheck_msg.h
CATFILES	= quota.cat quotaon.cat quot.cat repquota.cat edquota.cat \
		  quotacheck.cat
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

quotaon_OFILES	= quotaon.o dqutil.o fstab.o
edquota_OFILES	= edquota.o dqutil.o fstab.o
quota_OFILES	= quota.o dqutil.o fstab.o
quot_OFILES	= quot.o dqutil.o fstab.o
repquota_OFILES	= repquota.o dqutil.o fstab.o
quotacheck_OFILES = quotacheck.o dqutil.o fstab.o

LIBS		= -lfs

.include <${RULES_MK}>

