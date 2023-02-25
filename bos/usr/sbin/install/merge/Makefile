# @(#)76        1.3  src/bos/usr/sbin/install/merge/Makefile, cmdinstl, bos411, 9428A410j 6/3/94 11:55:36
#-----------------------------------------------------------------------
#
#   COMPONENT_NAME: cmdinstl
#
# FUNCTIONS: Makefile (merge_41vpds)
#
#   ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#   (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------------

PROGRAMS        = merge_41vpds
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

ILIST           =merge_41vpds
IDIR            =/usr/lpp/bos/migrate/

INCFLAGS        = -I../messages -I../include

CC_OPT_LEVEL    =

OFILES            = merge_41vpds.o

LIBS=  -lodm -lc -ls -lPW -lcfg -lIN -lswvpd -linulib

.include <${RULES_MK}>
