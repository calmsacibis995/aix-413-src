# @(#) 60 1.17 src/packages/bos/bosinst/Makefile, bosinst, bos412, GOLDA411a 9/8/94 18:17:35
#
#   COMPONENT_NAME: BOSINST
#
#   FUNCTIONS: 
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


VPATH		= ${MAKETOP}/bos/diag/diags:${MAKETOP}/bos/usr/lpp/bosinst/tape:${MAKETOP}/packages/bos/rte/usr:${MAKETOP}/bos/usr/lpp/bosinst/data

SCRIPTS		= dskpt.dep dskpt.stz tapefiles1 bos.rte.cfgfiles \
		  bos.rte.usr.rmlist incompat.pkgs bos.rte.pre_i \
		  bos.rte.root.rmlist

ILIST		= bosinst.tape bosinst.tape.client  bosinst.tape.eserver
IDIR		= /

OTHERS		= bosinst.tape  bosinst.tape.client bosinst.tape.eserver

CLEANFILES	= bosinst.tape bosinst.tape.client  bosinst.tape.eserver \
			tapefiles1 dskpt.add

dskpt.add: dskpt.stz dskpt.dep
	${RM} -f dskpt.add
	for i in `${CAT} dskpt.stz` ; do \
	${CAT} `${ECHO} $$i | ${FINDSHIPFILE}` >> ${.TARGET}; \
	done;

bosinst.tape: tapefiles1 dskpt.add bos.rte.cfgfiles bos.rte.usr.rmlist \
		incompat.pkgs bos.rte.pre_i bos.rte.root.rmlist \
		bosinst.data.full
	# Copy migration files into bosinst.tape image directory structure.
	${MAKEPATH} usr/lpp/bos/foo
	${RM} -f usr/lpp/bos/foo
	${CP} bos.rte.cfgfiles bos.rte.usr.rmlist incompat.pkgs bos.rte.pre_i \
		bos.rte.root.rmlist usr/lpp/bos
	# Strip libraries.
	${MAKEPATH} usr/lib/foo
	${RM} -f usr/lib/foo
	${MAKEPATH} usr/ccs/lib/foo
	${RM} -f usr/ccs/lib/foo
	${MAKEPATH} usr/lpp/diagnostics/lib/foo
	${RM} -f usr/lpp/diagnostics/lib/foo
	${GREP} -h -e /usr/lib/lib -e /usr/lpp/diagnostics/lib/lib \
		-e usr/ccs/lib/lib dskpt.dep tapefiles1 | \
		while read libname rest; do \
			${RM} -f shr.o .$$libname; \
			${AR} -x `${ECHO} $$libname | ${FINDSHIPFILE}` shr.o; \
			${STRIP} shr.o; \
			${AR} cq .$$libname shr.o; \
			${RM} -f shr.o; \
		done;
	# Copy BOS and diag dependency files from ship tree.
	${GREP} -h -v /usr/lib/lib tapefiles1 dskpt.dep \
		| ${GREP} -h -v /usr/ccs/lib/lib \
		| ${GREP} -h -v /usr/lpp/diagnostics/lib/lib \
		| ${GREP} -v BosMenus.cat \
		| ${GREP} -v /usr/bin/ar \
			| while read fn; \
			do \
				${MAKEPATH} .$$fn; \
				${CP} `${ECHO} $$fn | ${FINDSHIPFILE}` .$$fn; \
			done;
	/usr/bin/ln -fs /usr/ccs/bin/ar ./usr/bin/ar
	# Add diagnostics definitions to device ODM.
	(ODMDIR=./usr/lib/objrepos export ODMDIR; ${ODMADD} dskpt.add)
	# Copy all available bosinst message catalogs from ship tree.
	${RM} -f usr/lib/nls/msg/*/BosMenus.cat
	# First look in the sandbox for all catalogs and then in the backing
	# tree.  Test to ensure the catalog really exists before trying to copy.
	# Then test to ensure it isn't already in package directory structure.
	# Then get determine if the locale has translated the catalog by looking
	# in the ILS database.  If all of those are met, copy it to the package
	# directory structure.
	for i in `${ECHO} ${SHIP_PATH} | ${TR} ':' ' '`; do \
		for j in $$i/usr/lib/nls/msg/*/BosMenus.cat; do \
			if test ! -f $$j; then continue; fi; \
			j=`${ECHO} $$j | ${SED} "s:$$i/::"`; \
			if test -f $$j; then continue; fi; \
			lang=`/usr/bin/dirname $$j`; \
			lang=`${BASENAME} $$lang`; \
			isitin=`(ODMDIR=usr/lib/objrepos ${ODMGET} -qlocale=$$lang MESSAGES | ${GREP} bosinst_translated | ${SED} -e 's/^.*bosinst_translated = "//' -e 's/"//g'  -e 's/[	 ]//g')`; \
			if test X$$isitin != Xy; then continue; fi; \
			${MAKEPATH} $$j; \
			${CP} $$i/$$j $$j; \
		done; \
	done;
	# WARNING:  migration files are included in the product bosinst.tape
	# image, but not in the tapefiles1 which creates the mksysb bosinst.tape
	# image.  This is because it is not possible to boot a mksysb tape and
	# do a migration install.
	${CP} ${bosinst.data.full:P} bosinst.data
	{ /usr/bin/ls -1 ./usr/lib/nls/msg/*/BosMenus.cat ./usr/lpp/bos/*; \
		${CAT} tapefiles1 dskpt.dep; } | ${SED} -e 's:^/:\./:' -e '/\*/d' \
			| ${BACKUP} -ivrf bosinst.tape -b30;

bosinst.tape.client: bosinst.tape bosinst.data.client
	${CP} ${bosinst.data.client:P} bosinst.data
	{ /usr/bin/ls -1 ./usr/lib/nls/msg/*/BosMenus.cat ./usr/lpp/bos/*; \
		${CAT} tapefiles1 dskpt.dep; } | ${SED} -e 's:^/:\./:' -e '/\*/d' \
			| ${BACKUP} -ivrf bosinst.tape.client -b30;

bosinst.tape.eserver: bosinst.tape bosinst.data.eserver
	${CP} ${bosinst.data.eserver:P} bosinst.data
	{ /usr/bin/ls -1 ./usr/lib/nls/msg/*/BosMenus.cat ./usr/lpp/bos/*; \
		${CAT} tapefiles1 dskpt.dep; } | ${SED} -e 's:^/:\./:' -e '/\*/d' \
			| ${BACKUP} -ivrf bosinst.tape.eserver -b30;

#	@${RM} -fr sbin etc usr bosinst.data image.data

.include <${RULES_MK}>
