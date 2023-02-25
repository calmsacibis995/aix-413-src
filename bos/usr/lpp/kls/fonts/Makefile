# @(#)79	1.6  src/bos/usr/lpp/kls/fonts/Makefile, cmdkr, bos411, 9428A410j 3/24/94 10:29:13
#
#   COMPONENT_NAME: cmdkr
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS = \
	misc

FONT_LIST = \
	EnglHg16.pcf.Z \
	EnglHg24.pcf.Z \
	Hangul16.pcf.Z \
	Hangul24.pcf.Z \
	HGPRT16.pcf.Z \
	HGPRT24.pcf.Z \
	HGPRT32.pcf.Z

SCRIPTS	= fonts.alias.KR

OTHERS	= ${FONT_LIST}

ILIST	= ${SCRIPTS} ${OTHERS}
IDIR	= /usr/lpp/X11/lib/X11/fonts/

.include <${RULES_MK}>

${FONT_LIST:M*.pcf.Z} :	$${.TARGET:.pcf.Z=.bdf}
	${RM} -f ${.TARGET} ${.TARGET:.Z=}
	${BDFTOPCF} ${.ALLSRC} >${.TARGET:.Z=}
	${COMPRESS} ${.TARGET:.Z=}
