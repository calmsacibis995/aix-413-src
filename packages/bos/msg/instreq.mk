# @(#)59	1.4  src/packages/bos/msg/instreq.mk, pkgmsg, pkg411, GOLD410 4/20/94 15:18:37
#
#   COMPONENT_NAME: pkgmsg
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   This .mk file can be used by any message packaging makefile that
#   desires to have an instreq to the appropriate code package 
#   automatically inserted into the package.....
#

PREREQ_LIST	= ${OPTIONS:=.prereq}
INFO_FILES	+= ${PREREQ_LIST}
BUILT_PREREQ_LIST	= ${PREREQ_LIST:S-^-usr/-g}

.include <${RULES_MK}>

${BUILT_PREREQ_LIST}:	${ALWAYS}
	@${MAKEPATH} ${.TARGET}
	${ECHO} "*instreq ${${.TARGET:S-^usr/${BFF}.--g:R}_INSTREQ:U${.TARGET:T:R:S/.msg.${MSGLANG}//}} ${LPPVERSION}.${LPPRELEASE}.${LPPMAINT}.${LPPFIXLEVEL}" > ${.TARGET}

.if (${MSGLANG} != "en_US")
.include <${MAKETOP}/packages/bos/msg/pkgmsg.mk>
.endif
