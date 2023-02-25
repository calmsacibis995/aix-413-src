# @(#)06	1.5  src/tcpip/usr/samples/tcpip/onhost/onhost.mk, tcpip, tcpip411, GOLD410 2/13/94 15:01:12
# 
#  COMPONENT_NAME: TCPIP onhost.mk
# 
#  FUNCTIONS:
# 
#  ORIGINS: 27
# 
#  (C) COPYRIGHT International Business Machines Corp. 1986, 1988, 1989
#  All Rights Reserved
#  Licensed Materials - Property of IBM
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
#
# INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
# EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
# WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
# LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
# OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
# IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
# DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
# DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
# CORRECTION.
#
#  RISC System/6000 is a trademark of International Business Machines
#   Corporation.
#
# 
# Makefile for hostconnect and onhost, version 1.1
# AIXwhat is onhost.mk 1.5 PASC 1.5
#
.SUFFIXES: .msg .cat
IDENT=
 
CFLAGS=	-O -c
 
all: $(ALL)

SOURCE	= hostcon0.c hostcon1.c hostcon2.c
OBJS	= hostcon0.o hostcon1.o hostcon2.o
HFILES	= hostcon0.h onhost0.h
MSG	= hostconn.msg onhost.msg
CAT	= hostconn.cat onhost.cat
MSGH	= hostconn_msg.h onhost_msg.h
 
SOURCE1	= onhost0.c
OBJS1	= onhost0.o
 
hostcon0.o hostcon1.o hostcon2.o : hostcon0.h onhost0.h
onhost0.o : onhost0.h
	
.msg.cat: $(MSG)
	runcat $* $*.msg

msgcat: $(CAT) $(MSGH)
	
.c.o:
	$(CC) $(DFLAGS) $(CFLAGS) $<   
	
hostconnect: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o hostconnect
	
onhost:	$(OBJS1)
	$(CC) $(OBJS1) $(LIBS) -o onhost
	
lint: $(SOURCE) $(HFILES)
	lint  $(DFLAGS) $(SOURCE) > lintlog
	
lint1: $(SOURCE1)
	lint $(DFLAGS) $(SOURCE1) $(LIBS) > lintlog1
	
print: $(SOURCE) $(HFILES)
	ctags -x $(SOURCE) > index
	pr -f -n  index $(HFILES) $(SOURCE) | print
	
print1:	$(SOURCE1) $(HFILES)
	ctags -x $(SOURCE1) > index1
	pr -f -n  index1 $(SOURCE1) | print
	
xtags:	$(SOURCE) $(SOURCE1)
	ctags -x $(SOURCE) > index
	ctags -x $(SOURCE1) > index1
	
tags:	$(HFILES) $(SOURCE)
	ctags $(HFILES) $(SOURCE)
	
tags1:	$(SOURCE1)
	-if [ -s tags ] \
	;then mv tags tags2 \
	;fi
	ctags $(SOURCE1)
	mv tags tags1
	-if [ -s tags2 ] \
	;then mv tags2 tags \
	;fi
	
vers:	$(HFILES) $(SOURCE) $(SOURCE1)
	rm -f onhost.vers
	echo `date` > onhost.vers
	grep @\(#\) $(HFILES) $(SOURCE) $(SOURCE1) >> onhost.vers
	grep @\(#\) onhost.make onhost.mk onhost.profil >> onhost.vers
	grep @\(#\) *.l readme.aix *.cms *.tso *.msg>> onhost.vers
