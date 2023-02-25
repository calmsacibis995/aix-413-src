# @(#)38        1.7  src/bos/usr/sbin/acct/Makefile, cmdacct, bos411, 9428A410j 5/6/93 16:39:37
# COMPONENT_NAME: CMDACCT
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

SUBDIRS		= ac lib tmplts

PROGRAMS	= acctcms acctcom acctcon1 acctcon2 acctdisk acctdusg \
		  acctmerg accton acctprc1 acctprc2 acctwtmp fwtmp wtmpfix

SCRIPTS		= chargefee ckpacct dodisk lastlogin nulladm prctmp remove \
		  prdaily prtacct runacct shutacct startup turnacct monacct

DATAFILES	= ptecms.awk ptelus.awk

MSGHDRS		= acct_msg.h

CATFILES	= acct.cat

HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS} ${SCRIPTS} ${DATAFILES} ctmp.h ptmp.h tacct.h

IDIR		= /usr/sbin/acct/

ctmp.h_IDIR	= /usr/include/sys/

ptmp.h_IDIR	= /usr/include/sys/

tacct.h_IDIR	= /usr/include/sys/

acctmerg.o_INCFLAGS	= -I${MAKETOP}/bos/usr/bin/que

LIBFLAGS	= -Llib

LIBS		= -la

acctdusg_LIBS   = -la -ls

.include <${RULES_MK}>
