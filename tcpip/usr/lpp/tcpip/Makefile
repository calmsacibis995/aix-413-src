# @(#)43	1.6  src/tcpip/usr/lpp/tcpip/Makefile, tcpip, tcpip41J, 9519A_all 5/9/95 16:34:29
# 
# COMPONENT_NAME: TCPIP Makefile
# 
# FUNCTIONS: 
#
# ORIGINS: 10  26  27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1994
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Licensed Materials - Property of IBM
#

VPATH			= ${TARGET_MACHINE}
DATAFILES		= sm_tcpip1.add sm_tcpip2.add

ILIST			= ${DATAFILES}
IDIR			= /usr/lpp/tcpip/
IMODE			= 644

NOUPDATE_LIST		= \
			/usr/lpp/tcpip/sm_tcpip1.add \
			/usr/lpp/tcpip/sm_tcpip2.add

.include <${RULES_MK}>

.SUFFIXES: .add .addN

.addN.add:
	@${RM} -f ${.TARGET}
	${BRAND} ${BRANDDICT} < ${.IMPSRC} > ${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
