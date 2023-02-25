# @(#)76	1.8  src/tenplus/hlib/help/Makefile, tenplus, tenplus411, GOLD410 10/27/93 11:18:34
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


VPATH = ${MAKETOP}/tenplus/e2/common:${MAKETOP}/tenplus/interface
SCRIPTS = helper.in.S
LIBRARIES = libhelp.a 

OFILES =  Uafter.o Udel.o Urestart.o himports.o Ualarm.o Uins.o		\
          Usavestate.o help.o pipe.o Ubefore.o Umod.o Ustop.o 

CFLAGS =  -DBLTHELP -DPIPEHELP 
himports.o_CFLAGS = -DBLTHELP 
INCFLAGS = -I${MAKETOP}/tenplus/include -I${MAKETOP}/tenplus/e2/include

.include <${RULES_MK}>
