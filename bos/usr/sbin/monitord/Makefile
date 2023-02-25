# @(#)91	1.1  src/bos/usr/sbin/monitord/Makefile, netls, bos411, 9428A410j 4/2/94 14:46:15
#
#   COMPONENT_NAME: netls
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Makefile


PROGRAMS        = monitord

ILIST           = monitord

MSGHDRS		= monitord_admin_msg.h

CATFILES	= monitord_admin.cat

HFILES		= ${MSGHDRS}

IDIR     	= /usr/sbin/

IMODE    	= 755

LIBS     	= -lnetls -lnck

OFILES   	= monitord.o monitord_svr.o monitord_netls.o

.include <${RULES_MK}>
