#
# COMPONENT_NAME: DBAUD
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

VPATH		= ${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}:${MAKETOP}/packages/bos/diagboot:${MAKETOP}/bos/objclass/power:${MAKETOP}/../ship/power/etc/drivers


SCRIPTS		= diagS.dep diagstartS createshipfile.awk

DSKTIMAGE	= diagS.image
CLEANFILES	= diagS.image
ADDFILES	= dbaud.add
OTHERS		= diagS.contents ${ADDFILES} ${DSKTIMAGE}

.include <${RULES_MK}>

diagS.contents: diagS.dep
	${RM} -f ${.TARGET}
	(for i in `${CAT} diagS.dep`; do \
	if [ "$$i" != "./etc/diagstartS" -a "$$i" != "./etc/diagS.dep" \
	-a "$$i" != "./etc/stanzas/dbaud.add" ]; then\
	${ECHO} $$i | ${SED} -e 's,.,,'  >> ${.TARGET}; fi; done)

dbaud.add:  diag.dfe5.add baud.add
	${RM} -f ${.TARGET}
	${CP} ${baud.add:P} ${.TARGET}
	${CHMOD} +w ${.TARGET}
	${CAT} ${diag.dfe5.add:P} >> ${.TARGET}

.undef  DIAG_DSKT_LIST
diagS.image: diagstartS diagS.contents \
	copyshipfile.awk createshipfile.awk \
	${DIAG_DSKT_LIST:!if [ -f diagS.contents ] ; then \
	for i in `${CAT} diagS.contents`; do \
	${ECHO} `${ECHO} $$i | ${FINDSHIPFILE}`; done; fi!}
	for i in `${CAT} diagS.dep`; do \
	if [ "$$i" = "./etc/diagstartS" -o "$$i" = "./etc/diagS.dep" \
	-o "$$i" = "./etc/stanzas/dbaud.add" ] ; \
	then ${ECHO} $$i | ${AWK} -f ${createshipfile.awk:P}; \
	else ${ECHO} $$i | ${AWK} -f ${copyshipfile.awk:P}; fi; done;
	${CPIO} -oudC36 < diagS.dep > ${.TARGET}
	for i in `${CAT} diagS.dep` ; do \
	${RM} -rf $$i; \
	done;
