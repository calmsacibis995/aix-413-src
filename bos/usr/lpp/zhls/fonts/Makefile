# @(#)44	1.3  src/bos/usr/lpp/zhls/fonts/Makefile, ils-zh_CN, bos41J, 9512A_all 3/16/95 21:15:17
#
# COMPONENT_NAME:  ILS-ZH_CN
#
# FUNCTIONS: Simplified Chinese Default Fonts
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

FONT_LIST = \
	24x48.pcf.Z \
	26x54romucs.pcf.Z	\
	52x54gbucs.pcf.Z	\
	52x54restucs.pcf.Z	\
	52x54t1ucs.pcf.Z	\
	52x54t2ucs.pcf.Z	\
	Roman27ucs.pcf.Z	\
	cns1ucs.pcf.Z	\
	cns2ucs.pcf.Z	\
	gb27ucs.pcf.Z	\
	rest27ucs.pcf.Z	\
	udc27ucs.pcf.Z



OTHERS	= ${FONT_LIST}

SCRIPTS = fonts.alias.ZH

ILIST   = ${SCRIPTS} ${OTHERS}

IDIR    = /usr/lpp/X11/lib/X11/fonts/

.include <${RULES_MK}>

${FONT_LIST:M*.pcf.Z} : $${.TARGET:.pcf.Z=.bdf}
	${RM} -f ${.TARGET} ${.TARGET:.Z=}
	${BDFTOPCF} ${.ALLSRC} >${.TARGET:.Z=}
	${COMPRESS} ${.TARGET:.Z=}
