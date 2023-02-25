# @(#)11	1.10  src/tenplus/keys/Makefile, tenplus, tenplus41J, 9517A_all 4/24/95 21:10:28
#
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10, 27
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1995
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


PROGRAMS	= keys 
OTHERS = keys_executed
INCFLAGS = -I${MAKETOP}/tenplus/include
LIBS = -lutil -lobj 
LIBFLAGS = -L${MAKETOP}/tenplus/lib/util -L${MAKETOP}/tenplus/lib/obj

.include <${RULES_MK}>

ed.keys: keys.stz
	${TOSF} ${keys.stz:P}

keys.frm: keys.fsz
	${TOSF} ${keys.fsz:P}

keysz.frm: keysz.fsz
	${TOSF} ${keysz.fsz:P}

tables.c keynames.h names.h readkeys:
	${RM} -f ${.TARGET}
	${TOUCH} -f ${.TARGET}

keys_executed: keys ed.keys tables.c keynames.h names.h readkeys
	${RM} -f ${.TARGET} tables.c keynames.h names.h readkeys
	${KEYS} ${ed.keys:P}
	${TOUCH} ${.TARGET}
