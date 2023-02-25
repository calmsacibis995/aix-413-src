# @(#)97        1.10  src/packages/bos/rte/rte/Makefile, pkgbossub, pkg41J, 9524E_all 6/13/95 08:56:13
#
# COMPONENT_NAME: pkgbossub
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ..:root

BFF		= bos

OPTIONS		= bos.rte

ROOT_OPTIONS	= bos.rte

FILESET_BASE	= rte
SCRIPTS		= bos.rte.add
OTHERS		= swvpd.make swvpdroot.make

BIG_IL		= ${MAKETOP}packages/bos/rte/bos.rte.il
CR_FILE		= ${MAKETOP}packages/bos/rte/bos.rte.cr
LP_FILE		= ${MAKETOP}packages/bos/rte/bos.rte.lp

ROOT_LIBLPP_UPDT_LIST	+= bos.rte.post_u

# Builds an inslist for this fileset
bos.rte.il : ${BIG_IL}
	@-${RM} -f ${.TARGET}
	${AWK} '$$1 !~ /^#/ && $$NF == FSET { print }' FSET=${FILESET_BASE} \
		${.ALLSRC} > ${.TARGET} || ${RM} -f ${.TARGET}

# Copy down bos.rte.cr to {OPTION}.cr
bos.rte.cr : ${CR_FILE}
	@-${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

# Copy down bos.rte.lp to {OPTION}.lp
bos.rte.lp : ${LP_FILE}
	@-${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

# Add vpd information in the swvpd databases
swvpd.make : bos.rte.add INVENTORY_LIST \
		${MAKETOP}packages/bos/rte/usr/lib/objrepos/swvpd.usr.make
	ODMDIR=${MAKETOP}packages/bos/rte/usr/lib/objrepos ${ODMADD} ${bos.rte.add:P}
	${ADDVPD} -f usr/bos.rte.inventory -l bos.rte -d ${MAKETOP}packages/bos/rte/usr/lib/objrepos
	${TOUCH} swvpd.make

# Add vpd information in the swvpd databases
swvpdroot.make : bos.rte.add INVENTORY_LIST \
		${MAKETOP}packages/bos/rte/etc/objrepos/swvpd.root.make
	ODMDIR=${MAKETOP}packages/bos/rte/etc/objrepos ${ODMADD} ${bos.rte.add:P}
	${ADDVPD} -f root/bos.rte.inventory -l bos.rte -d ${MAKETOP}packages/bos/rte/etc/objrepos
	${TOUCH} swvpdroot.make

.include <packdep.mk>
.include <${RULES_MK}>

usr/bos.rte.xref:
	${TOUCH} ${.TARGET}
