# @(#)73        1.27  src/packages/bos/bosboot/Makefile, bosboot, bos41J, 9519A_all 5/8/95 16:23:43
#
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS: create boot images in the build environment
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SCRIPTS		= proto.awk

BOOTLIBS	= libodm.a libcfg.a

ILIST		= cdfs.proto bosboot.cd bosboot.tape \
			bosboot_up.cd.rspc bosboot_up.cd.rspc.DB \
			bosboot_up.tape.rspc.DB
IDIR		= /

CLEANFILES	= ${BOOTLIBS} buildcd.proto buildtape.proto bosboot.cd \
			bosboot.tape rspc.buildcd.proto \
			bosboot_up.cd.rspc bosboot_up.cd.rspc.DB \
			bosboot_up.tape.rspc.DB

OTHERS = \
	bosboot.cd \
	bosboot.tape \
	bosboot_up.cd.rspc \
	bosboot_up.cd.rspc.DB \
	bosboot_up.tape.rspc.DB \
	cdfs.proto

PROTO_CPP_FLAGS	= -P

.if exists (${MAKETOP}ssa)
PROTO_CPP_FLAGS	+= -DSSA
.endif

.include <${RULES_MK}>

cdfs.proto:
	${CPP} ${PROTO_CPP_FLAGS} ${cdfs.proto.S:P} > ${.TARGET}

${BOOTLIBS}:
	${RM} -f shr.o ${.TARGET}
	${AR} -x `${ECHO} /usr/lib/${.TARGET} | ${FINDSHIPFILE}` shr.o
	${STRIP} shr.o
	${AR} cq ${.TARGET} shr.o
	${RM} shr.o

bootrec:
	${TOUCH} ${.TARGET}

cd.proto:
	${CPP} ${PROTO_CPP_FLAGS} ${cd.proto.S:P} > ${.TARGET}

buildcd.proto: cd.proto proto.awk ${BOOTLIBS}
	${AWK} -f ${proto.awk:P} outfile=${.TARGET} ${cd.proto:P} || \
	( ${RM} ${.TARGET}; exit 1 )

rspc.cd.proto:
	${CPP} ${PROTO_CPP_FLAGS} ${rspc.cd.proto.S:P} > ${.TARGET}

rspc.buildcd.proto: rspc.cd.proto proto.awk ${BOOTLIBS}
	${AWK} -f ${proto.awk:P} outfile=${.TARGET} ${rspc.cd.proto:P} || \
	( ${RM} ${.TARGET}; exit 1 )

rspc.tape.proto:
	${CPP} ${PROTO_CPP_FLAGS} ${rspc.tape.proto.S:P} > ${.TARGET}

rspc.buildtape.proto: rspc.tape.proto proto.awk ${BOOTLIBS}
	${AWK} -f ${proto.awk:P} outfile=${.TARGET} ${rspc.tape.proto:P} || \
	( ${RM} ${.TARGET}; exit 1 )

tape.proto:
	${CPP} ${PROTO_CPP_FLAGS} ${tape.proto.S:P} > ${.TARGET}

buildtape.proto: tape.proto proto.awk ${BOOTLIBS}
	${AWK} -f ${proto.awk:P} outfile=${.TARGET} ${tape.proto:P} || \
	( ${RM} ${.TARGET}; exit 1 )

cd.fs:	buildcd.proto bootrec
	${TOUCH} ${.TARGET}
	${ECHO} y | ${MKFS} -V jfs -p ${buildcd.proto:P} ${.TARGET}

rspc.cd.fs:	rspc.buildcd.proto bootrec
	${TOUCH} ${.TARGET}
	${ECHO} y | ${MKFS} -V jfs -p ${rspc.buildcd.proto:P} ${.TARGET}

rspc.tape.fs:	rspc.buildtape.proto bootrec
	${TOUCH} ${.TARGET}
	${ECHO} y | ${MKFS} -V jfs -p ${rspc.buildtape.proto:P} ${.TARGET}

tape.fs:	buildtape.proto bootrec
	${TOUCH} ${.TARGET}
	${ECHO} y | ${MKFS} -V jfs -p ${buildtape.proto:P} ${.TARGET}

cd.ramfs:	cd.fs
	${MKRAM} ${cd.fs:P} ${.TARGET}

rspc.cd.ramfs:	rspc.cd.fs
	${MKRAM} ${rspc.cd.fs:P} ${.TARGET}

rspc.tape.ramfs:	rspc.tape.fs
	${MKRAM} ${rspc.tape.fs:P} ${.TARGET}

tape.ramfs:	tape.fs
	${MKRAM} ${tape.fs:P} ${.TARGET}

bosboot_up.tape.DB: tape.ramfs
	${TOUCH} /tmp/tapeboot.build_dev.DB
	${MKBOOT} -d/tmp/tapeboot.build_dev.DB -Ibk`${ECHO} /unix_up.strip | \
		${FINDSHIPFILE}` -f/tmp/tapeboot.build_dev.DB | ${COMPRESS} > \
		/tmp/tapeboot.build_image.DB
	${CAT} ${tape.ramfs:P} >> /tmp/tapeboot.build_image.DB
	${MKBOOT} -d/tmp/tapeboot.build_image.DB -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-f/tmp/tapeboot.build_image.DB > bosboot_up.tape.DB
	${RM} /tmp/tapeboot.build_image.DB /tmp/tapeboot.build_dev.DB

bosboot.cd: cd.ramfs
	${TOUCH} /tmp/cdboot.build_dev
	${MKBOOT} -d/tmp/cdboot.build_dev -Dbk`${ECHO} /unix_mp.strip | \
		${FINDSHIPFILE}` -f/tmp/cdboot.build_dev | ${COMPRESS} > \
		/tmp/cdboot.build_image
	${CAT} ${cd.ramfs:P} >> /tmp/cdboot.build_image
	${MKBOOT} -d/tmp/cdboot.build_image -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-f/tmp/cdboot.build_image > bosboot.cd
	${RM} /tmp/cdboot.build_image /tmp/cdboot.build_dev

bosboot.tape: tape.ramfs
	${TOUCH} /tmp/tapeboot.build_dev
	${MKBOOT} -d/tmp/tapeboot.build_dev -Dbk`${ECHO} /unix_mp.strip | \
		${FINDSHIPFILE}` -f/tmp/tapeboot.build_dev | ${COMPRESS} > \
		/tmp/tapeboot.build_image
	${CAT} ${tape.ramfs:P} >> /tmp/tapeboot.build_image
	${MKBOOT} -d/tmp/tapeboot.build_image -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-f/tmp/tapeboot.build_image > bosboot.tape
	${RM} /tmp/tapeboot.build_image /tmp/tapeboot.build_dev

bosboot.tape.DB: tape.ramfs
	${TOUCH} /tmp/tapeboot.build_dev.DB
	${MKBOOT} -d/tmp/tapeboot.build_dev.DB -Ibk`${ECHO} /unix_mp.strip | \
		${FINDSHIPFILE}` -f/tmp/tapeboot.build_dev.DB | ${COMPRESS} > \
		/tmp/tapeboot.build_image.DB
	${CAT} ${tape.ramfs:P} >> /tmp/tapeboot.build_image.DB
	${MKBOOT} -d/tmp/tapeboot.build_image.DB -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-f/tmp/tapeboot.build_image.DB > bosboot.tape.DB
	${RM} /tmp/tapeboot.build_image.DB /tmp/tapeboot.build_dev.DB

bosboot_up.cd.rspc: rspc.cd.ramfs
	${TOUCH} /tmp/rspc.cdboot.build_dev
	${MKBOOT} -d/tmp/rspc.cdboot.build_dev -Dbk`${ECHO} /unix_up.strip | \
		${FINDSHIPFILE}` -f/tmp/rspc.cdboot.build_dev | ${COMPRESS} > \
		/tmp/rspc.cdboot.build_image
	${CAT} ${rspc.cd.ramfs:P} >> /tmp/rspc.cdboot.build_image
	${MKBOOT} -d/tmp/rspc.cdboot.build_image -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-A `${ECHO} /usr/lib/boot/aixmon | ${FINDSHIPFILE}` \
		-f/tmp/rspc.cdboot.build_image > bosboot_up.cd.rspc
	${RM} /tmp/rspc.cdboot.build_image /tmp/rspc.cdboot.build_dev

bosboot_up.cd.rspc.DB: rspc.cd.ramfs
	${TOUCH} /tmp/rspc.cdboot.build_dev.DB
	${MKBOOT} -d/tmp/rspc.cdboot.build_dev.DB -Ibk`${ECHO} /unix_up.strip \
		| ${FINDSHIPFILE}` -f/tmp/rspc.cdboot.build_dev.DB | \
		${COMPRESS} > /tmp/rspc.cdboot.build_image.DB
	${CAT} ${rspc.cd.ramfs:P} >> /tmp/rspc.cdboot.build_image.DB
	${MKBOOT} -d/tmp/rspc.cdboot.build_image.DB -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-A `${ECHO} /usr/lib/boot/aixmon | ${FINDSHIPFILE}` \
		-f/tmp/rspc.cdboot.build_image.DB > bosboot_up.cd.rspc.DB
	${RM} /tmp/rspc.cdboot.build_image.DB /tmp/rspc.cdboot.build_dev.DB

bosboot_up.tape.rspc.DB: rspc.tape.ramfs
	${TOUCH} /tmp/rspc.tapeboot.build_dev.DB
	${MKBOOT} -d/tmp/rspc.tapeboot.build_dev.DB \
		-Ibk`${ECHO} /unix_up.strip \
		| ${FINDSHIPFILE}` -f/tmp/rspc.tapeboot.build_dev.DB | \
		${COMPRESS} > /tmp/rspc.tapeboot.build_image.DB
	${CAT} ${rspc.tape.ramfs:P} >> /tmp/rspc.tapeboot.build_image.DB
	${MKBOOT} -d/tmp/rspc.tapeboot.build_image.DB -bisk \
		`${ECHO} /usr/sbin/bootexpand | ${FINDSHIPFILE}` \
		-A `${ECHO} /usr/lib/boot/aixmon | ${FINDSHIPFILE}` \
		-f/tmp/rspc.tapeboot.build_image.DB > bosboot_up.tape.rspc.DB
	${RM} /tmp/rspc.tapeboot.build_image.DB /tmp/rspc.tapeboot.build_dev.DB
