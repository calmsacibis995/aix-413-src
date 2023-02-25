# @(#)42	1.5  src/tcpip/usr/samples/tcpip/onhost/Makefile, tcpip, tcpip411, GOLD410 2/13/94 14:53:45
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Licensed Materials - Property of IBM
#

VPATH                   = ${TARGET_MACHINE}
DATAFILES		= hostcon0.c hostcon0.h hostcon1.c hostcon2.c \
			hostconn.msg hostconnect.man onhost.cms onhost.man \
			onhost.make onhost.mk onhost.msg onhost.profil \
			onhost.tso onhost0.c onhost0.h onhostcp.cms \
			onhostcp.tso onhoste.cms onhoste.tso onhostld.cms \
			onhostld.tso onhostmn.tso README.aix README.cms \
			README.tso

ILIST			= ${DATAFILES}
IDIR			= /usr/samples/tcpip/onhost/
IMODE			= 644

.include <${RULES_MK}>

#  Special Branding Rules
#
.cN.c:
	@${RM} -f ${.TARGET}
	${BRAND} ${BRANDDICT} < ${.IMPSRC} > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
