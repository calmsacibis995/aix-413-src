# @(#)38        1.5  src/packages/bos/rte/subsys.mk, pkgbossub, pkg41J, 9520A_c 5/18/95 16:22:30
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

# NOTE : the rte subsystem does NOT include this file...

# This comment has been added to ease in the tracking of the lpp_id
# used for the bos.rte update filesets.  The maximum is 50 before the
# swvpd.add.S file will need to update the CNTL.
#
# The last number used for lpp_id was 48 for X11
#
# skipped lpp_id = 20 when libg was removed.

VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass


# List of subsytems in bos.rte.  This list is used to create
# the list of add files for creating the proper dependencies
# in the etc/objrepos/Makefile and usr/lib/objrepos/Makefile
# files for the swvpd.make target.  It is also used in the
# src/packages/bos/rte/Makefile to complete the list of subdirectories
# to walk.
SUBSYSTEMS = rte Dt ILS SRC aio archive bind_cmds boot bosinst commands \
	  compare console control cron date devices devices_msg \
	  diag edit filesystem iconv im install libc libcfg libcur \
	  libdbm libnetsvc libpthreads libs loc lvm man methods \
	  misc_cmds net odm printers security serv_aid shell \
	  streams tty jfscomp libqb ifor_ls

.if exists(${MAKETOP}/../ship/power/usr/lib/objrepos/GAI)
SUBSYSTEMS += X11
.endif

# List of all the .add files for the subsystems of bos.rte.
# This list assumes that each subsystem (except for rte) only
# has one .add file and that file has the name bos.rte.<subsys>.add.S
# The rte subsystem has bos.rte.add.S not bos.rte.rte.add.S so we've
# got to special case this subsystem.
BOSRTE_ADDFILES = ${SUBSYSTEMS:Nrte:@SS@${MAKETOP}packages/bos/rte/${SS}/bos.rte.${SS}.add@} \
	${MAKETOP}packages/bos/rte/rte/bos.rte.add

ROOTOBJREPOS =	${MAKETOP}packages/bos/rte/etc/objrepos/swvpd.root.make
USROBJREPOS  =	${MAKETOP}packages/bos/rte/usr/lib/objrepos/swvpd.usr.make

# Make sure that if any files shipped for any of the filesets in bos.rte
# that we go ahead and rebuild the install image.
BIG_SFDEP = ${MAKETOP}packages/bos/rte/shipfile_dependency
${BIG_SFDEP} : ${SUBSYSTEMS:Nrte:@SS@${MAKETOP}packages/bos/rte/${SS}/shipfile_dependency@}
	${TOUCH} ${.TARGET}

${SUBSYSTEMS:Nrte:@SS@${MAKETOP}packages/bos/rte/${SS}/shipfile_dependency@}:
	${MAKEPATH} ${.TARGET}
	${TOUCH} ${.TARGET}


.if defined(FILESET_BASE)

BFF		= bos

BIG_IL		= bos.rte.il
CR_FILE		= bos.rte.cr



# Builds an inslist for this fileset
bos.rte.${FILESET_BASE}.il : ${BIG_IL}
	@-${RM} -f ${.TARGET}
	${AWK} '$$1 !~ /^#/ && $$NF == FSET { print }' FSET=${FILESET_BASE} \
		${.ALLSRC} > ${.TARGET} || ${RM} -f ${.TARGET}

# Copy down bos.rte.cr to {OPTION}.cr
bos.rte.${FILESET_BASE}.cr : ${CR_FILE}
	@-${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}


.if (${FILESET_BASE} != "jfscomp")

# NOTE : the jfscomp subsystem does NOT have a usr part
# so a special version of swvpd.make was needed in the
# jfscomp/Makefile.  Any changes to this rule should be
# copied to the jfscomp/Makefile

# Add vpd information in the swvpd databases
swvpd.make : bos.rte.${FILESET_BASE}.add INVENTORY_LIST ${USROBJREPOS} ${BIG_SFDEP}
	ODMDIR=${MAKETOP}packages/bos/rte/usr/lib/objrepos ${ODMADD} ${bos.rte.${FILESET_BASE}.add:P}
	${ADDVPD} -f usr/bos.rte.${FILESET_BASE}.inventory -l bos.rte.${FILESET_BASE} -d ${MAKETOP}packages/bos/rte/usr/lib/objrepos
	${TOUCH} swvpd.make

.endif # FILESET_BASE != "jfscomp"

.if defined(ROOT_OPTIONS)

swvpdroot.make : bos.rte.${FILESET_BASE}.add INVENTORY_LIST ${ROOTOBJREPOS} ${BIG_SFDEP}
	ODMDIR=${MAKETOP}packages/bos/rte/etc/objrepos ${ODMADD} ${bos.rte.${FILESET_BASE}.add:P}
	${ADDVPD} -f root/bos.rte.${FILESET_BASE}.inventory -l bos.rte.${FILESET_BASE} -d ${MAKETOP}packages/bos/rte/etc/objrepos
	${TOUCH} swvpdroot.make

.endif # defined ROOT_OPTIONS

.if defined(RTE_TCB)

# Add TCB files to sysck.cfg file
sysck.make : INVENTORY_LIST ${BIG_SFDEP}
	(${CAT} usr/bos.rte.${FILESET_BASE}.tcb 2>/dev/null; ${ECHO}) >> ${MAKETOP}packages/bos/rte/etc/security/sysck.cfg
	${TOUCH} sysck.make

.if defined(ROOT_OPTIONS)

# Add TCB files to sysck.cfg file
sysckroot.make : INVENTORY_LIST ${BIG_SFDEP}
	${RM} -f ${.TARGET} sysckroot.mk
	(${CAT} root/bos.rte.${FILESET_BASE}.tcb 2>/dev/null; ${ECHO}) >> ${MAKETOP}packages/bos/rte/etc/security/sysck.cfg
	${TOUCH} sysckroot.mk

.endif # defined ROOT_OPTIONS
.endif # defined RTE_TCB
.else  # defined FILESET_BASE

# Force the target directory to be creatd if it doesn't exist.  The default
# .S to nothing rule does not do this force, so this file must be included
# after RULES_MK for this rule to be effective.
.S:
	@if [ -n ""${.TARGET:M*/*} ]; then ${MAKEPATH} ${.TARGET}; fi
	${RM} ${_RMFLAGS_} ${.TARGET}
	${SED} "/^#/d" <${.IMPSRC} >${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
	${CHMOD} +x ${.TARGET}


.endif # defined FILESET_BASE
