# @(#)23	1.11  src/tenplus/sf_progs/Makefile, tenplus, tenplus411, GOLD410 10/28/93 13:02:22
#
# COMPONENT_NAME: (TENPLUS) INed Editor
#
# FUNCTIONS:
#
# ORIGINS:  9, 10, 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


PROGRAMS = scat tosf fromsf sf2mri mri2sf \
	ghost history readfile rmhist fixascii newfile	
					 
readfile_OFILES	= readfile.o tree.o
tosf_OFILES	= tosf.o af.o
LIBS		= -lobj
sf2mri_LIBS	= -lobj -lutil
ghost_LIBS	= -lobj -ls
readfile_LIBS	= -lobj -lutil
rmhist_LIBS	= -lobj -ls
fixascii_LIBS	= -lobj -ls
LIBFLAGS = -L${MAKETOP}/tenplus/lib/obj -L${MAKETOP}/tenplus/lib/util
INCFLAGS = -I${MAKETOP}/tenplus/include

MSGHDRS = \
	fixascii_msg.h \
	ghost_msg.h \
	versions_msg.h \
	newfile_msg.h \
	readfile_msg.h \
	rmhist_msg.h

HFILES = ${MSGHDRS}
CATFILES = fixascii.cat ghost.cat versions.cat newfile.cat \
           readfile.cat rmhist.cat

ghost_IDIR = /usr/bin/
history_IDIR = /usr/bin/
readfile_IDIR = /usr/bin/
rmhist_IDIR = /usr/bin/
newfile_IDIR = /usr/bin/
fixascii_IDIR = /usr/lib/INed/bin/

ILIST = history ghost readfile rmhist newfile fixascii

.include <${RULES_MK}>
