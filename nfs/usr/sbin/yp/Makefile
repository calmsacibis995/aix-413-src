# 
#
# ORIGINS: 27 
#
#
# 

SUBDIRS		=	mknetid revnetgroup 

SCRIPTS		=	\
	ypinit		\
	ypxfr_1perday	\
	ypxfr_2perday	\
	ypxfr_1perhour

ILIST	=	\
	makedbm		\
	mkalias		\
	mrgpwd		\
	newkey		\
	stdethers	\
	stdhosts	\
	udpublickey	\
	yppoll		\
	yppush		\
	ypset 		\
	ypxfr		\
	${SCRIPTS}
IDIR		= /usr/sbin/

INCLUDES        =	\
        ypdefs.h	\
	ypsym.h

ypdefs.h_IDIR	= /usr/include/
ypsym.h_IDIR	= /usr/include/

ypdefs.h_ILIST	= ypdefs.h
ypsym.h_ILIST	= ypsym.h

.include <${RULES_MK}>

