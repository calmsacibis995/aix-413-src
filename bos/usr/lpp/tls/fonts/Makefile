# @(#)80        1.9  src/bos/usr/lpp/tls/fonts/Makefile, cmdtw, bos411, 9437C411a 9/16/94 09:24:27
#
# COMPONENT_NAME:  cmdtw
#
# FUNCTIONS: Taiwan Default Fonts
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

FONT_LIST = \
	RomanCh12.pcf.Z \
	RomanCh17.pcf.Z \
	RomanCh23.pcf.Z \
	Chtp112.pcf.Z \
	Chtp117.pcf.Z \
	Chtp123.pcf.Z \
	Chtp212.pcf.Z \
	Chtp217.pcf.Z \
	Chtp223.pcf.Z \
	Chtp312.pcf.Z \
	Chtp317.pcf.Z \
	Chtp323.pcf.Z \
	Chtpc12.pcf.Z \
	Chtpc17.pcf.Z \
	Chtpc23.pcf.Z \
	Chtpd12.pcf.Z \
	Chtpd17.pcf.Z \
	Chtpd23.pcf.Z \
	Chtp412.pcf.Z \
        Chtp417.pcf.Z

SCRIPTS	= fonts.alias.TW

OTHERS	= ${FONT_LIST}

ILIST   = ${SCRIPTS} ${OTHERS}
IDIR    = /usr/lpp/X11/lib/X11/fonts/

.include <${RULES_MK}>

${FONT_LIST:M*.pcf.Z} : $${.TARGET:.pcf.Z=.bdf}
	${RM} -f ${.TARGET} ${.TARGET:.Z=}
	${BDFTOPCF} ${.ALLSRC} >${.TARGET:.Z=}
	${COMPRESS} ${.TARGET:.Z=}
