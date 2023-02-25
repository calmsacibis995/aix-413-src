# @(#)63	1.9  src/tenplus/util/Makefile, tenplus, tenplus411, GOLD410 6/8/93 13:25:19
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# This directory used to contain a tiny program that ended up being
# called e2, that looked at environment variables and then execed one of
# several different versions of the editor.  It is no longer here.


PROGRAMS = 	tdigest addrem showrem 
OTHERS = INed.FATAL.LOG 
tdigest_OFILES = tdigest.o inseqs.o
OFILES = ${tdigest_OFILES} addrem.o showrem.o

MSGHDRS = tdigest_msg.h addrem_msg.h showrem_msg.h
HFILES = ${MSGHDRS}
CATFILES = tdigest.cat addrem.cat showrem.cat
LIBS = -lobj -lutil -lc
LIBFLAGS = -L${MAKETOP}/tenplus/lib/obj -L${MAKETOP}/tenplus/lib/util

INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/keys

tdigest_IDIR = /usr/bin/
addrem_IDIR = /usr/lib/INed/bin/
showrem_IDIR = /usr/lib/INed/bin/
INed.FATAL.LOG_IDIR = /var/

ILIST = tdigest addrem showrem INed.FATAL.LOG

.include <${RULES_MK}>

INed.FATAL.LOG:
	echo \
If this file, /var/INed.FATAL.LOG, is present and writeable, then\\n\
fatal INed errors will be recorded here.  If this file is not present,\\n\
it will not be created by INed. > INed.FATAL.LOG

