# @(#)39	1.4  src/bos/usr/sbin/acct/tmplts/Makefile, cmdacct, bos411, 9428A410j 2/8/94 08:37:00
#
# COMPONENT_NAME: CMDACCT
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

OTHERS		= wtmp

ILIST		= ${OTHERS} holidays
IDIR		= /var/adm/
holidays_IDIR	= /etc/acct/

.include <${RULES_MK}>

${OTHERS}:
	${TOUCH} ${.TARGET}

