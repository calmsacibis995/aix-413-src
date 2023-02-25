# @(#)79	1.1  src/packages/bos/msg/pkgmsg.mk, pkgmsg, pkg411, GOLD410 2/9/94 14:37:10
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
#   These rules can be used by any makefile that needs to build a
#   set of .il, .lp, and .cr files for a translated language, based
#   on the en_US version of those packaging files....
#

${OPTIONS:=.il}:	$${.TARGET:S/${MSGLANG}/en_US/}
	${SED} "s/en_US/${MSGLANG}/g" ${.ALLSRC} > ${.TARGET}

${OPTIONS:=.cr}:	$${.TARGET:S/${MSGLANG}/en_US/}
	${SED} "s/en_US/${MSGLANG}/g" ${.ALLSRC} > ${.TARGET}

${OPTIONS:=.lp}:	$${.TARGET:S/${MSGLANG}/en_US/}
	${SED} "s/en_US/${MSGLANG}/g" ${.ALLSRC} | ${SED} "s/U.S. English/${LANGDESC}/g" > ${.TARGET}
