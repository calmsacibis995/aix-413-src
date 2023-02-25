# @(#)61	1.1  src/packages/bos/diagboot/supplemental/deff/Makefile, mpada, pkg411, 9439B411a 9/28/94 17:15:29
#
# COMPONENT_NAME: DAMPA
#
# FUNCTIONS: create diagnostics supplemental image
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}:${MAKETOP}/packages/bos/diagboot


SCRIPTS		= diagS.dep diagstartS createshipfile.awk

DSKTIMAGE	= diagS.image
CLEANFILES	= diagS.image
ADDFILES	= dmpa.add
OTHERS		= diagS.contents ${ADDFILES} ${DSKTIMAGE}

.include <${RULES_MK}>

diagS.contents: diagS.dep
	${RM} -f ${.TARGET}
	(for i in `${CAT} diagS.dep`; do \
	if [ "$$i" != "./etc/diagstartS" -a "$$i" != "./etc/diagS.dep" \
	-a "$$i" != "./etc/stanzas/dmpa.add" ]; then\
	${ECHO} $$i | ${SED} -e 's,.,,'  >> ${.TARGET}; fi; done)

dmpa.add:  diag.deff.add mpaa.add
	${RM} -f ${.TARGET}
	${CP} ${diag.deff.add:P} ${.TARGET}
	${CAT} ${mpaa.add:P} >> ${.TARGET}

.undef  DIAG_DSKT_LIST
diagS.image: diagstartS diagS.contents \
	copyshipfile.awk createshipfile.awk \
	${DIAG_DSKT_LIST:!if [ -f diagS.contents ] ; then \
	for i in `${CAT} diagS.contents`; do \
	${ECHO} `${ECHO} $$i | ${FINDSHIPFILE}`; done; fi!}
	for i in `${CAT} diagS.dep`; do \
	if [ "$$i" = "./etc/diagstartS" -o "$$i" = "./etc/diagS.dep" \
	-o "$$i" = "./etc/stanzas/dmpa.add" ] ; \
	then ${ECHO} $$i | ${AWK} -f ${createshipfile.awk:P}; \
	else ${ECHO} $$i | ${AWK} -f ${copyshipfile.awk:P}; fi; done;
	${CPIO} -oudC36 < diagS.dep > ${.TARGET}
	for i in `${CAT} diagS.dep` ; do \
	${RM} -rf $$i; \
	done;
