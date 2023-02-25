# @(#)89	1.39  src/msg/bos/bosmsg.mk, bosmsg, msg411, 9439B411a 9/29/94 11:22:32
#
#   COMPONENT_NAME: bosmsg
#
#   FUNCTIONS: bosmsg.mk
#
#   ORIGINS: 27
#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#   OBJECT CODE	ONLY SOURCE MATERIALS
#
#   (C)	COPYRIGHT International	Business Machines Corp.	1994
#   All	Rights Reserved
#   US Government Users	Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM	Corp.
#
# This file contains the general rules for building translated message
# catalogs.  The rules are identical for each language being built.
#

CATFILES = \
	ac.cat \
	acct.cat \
	acledit.cat \
	aclget.cat \
	aclput.cat \
	adb.cat	\
	addrem.cat \
	adf.cat	\
	alog.cat \
	apply.cat \
	ar.cat \
	arp.cat	\
	as.cat \
	asl.cat	\
	ate.cat	\
	audit.cat \
	auditbin.cat \
	auditcat.cat \
	auditcv.cat \
	auditpr.cat \
	auditslt.cat \
	auditstm.cat \
	autopush.cat \
	awk.cat	\
	backup.cat \
	banner.cat \
	basename.cat \
	battery.cat \
	bbyname.cat \
	bc.cat \
	bdiff.cat \
	bellmail.cat \
	bf.cat \
	bfs.cat	\
	biff.cat \
	bind.cat \
	bindprocessor.cat \
	bl_err.cat \
	bootinfo.cat \
	bootlist.cat \
	bosboot.cat \
	BosMenus.cat \
	bs.cat \
	bsh.cat	\
	cal.cat	\
	calendar.cat \
	cancel.cat \
	canonls.cat \
	captoinfo.cat \
	capture.cat \
	cat.cat	\
	catman.cat \
	cb.cat \
	cfgchk.cat \
	cfgif.cat \
	cfginet.cat \
	cfgmgr.cat \
	cflow.cat \
	checkcw.cat \
	checknr.cat \
	chekmain.cat \
	chgif.cat \
	chginet.cat \
	chgrp.cat \
	chhwkbd.cat \
	chlang.cat \
	chmod.cat \
	chnamsv.cat \
	chown.cat \
	chprtsv.cat \
	chrctcp.cat \
	chroot.cat \
	chservices.cat \
	chslip.cat \
	chsubserver.cat	\
	chtcb.cat \
	chtz.cat \
	chusrprof.cat \
	chvmode.cat \
	ciedd.cat \
	cksum.cat \
	ckwinsiz.cat \
	cmdassist.cat \
	cmdcfg.cat \
	cmddump.cat \
	cmderrlg.cat \
	cmdinstl_e.cat \
	cmdlvm.cat \
	cmdnfs.cat \
	cmdnim.cat \
	cmdps.cat \
	cmdtrace.cat \
	cmp.cat	\
	col.cat	\
	colcrt.cat \
	colrm.cat \
	comm.cat \
	compress.cat \
	comsat.cat \
	confer.cat \
	console.cat \
	cp.cat \
	cpio.cat \
	cpu_state.cat \
	crash.cat \
	cron.cat \
	csh.cat	\
	csplit.cat \
	ctools.cat \
	cut.cat	\
	cw.cat \
	cxref.cat \
	d370pc.cat \
	dacorv.cat \
	dartic.cat \
	dasync.cat \
	datape.cat \
	date.cat \
	dbx.cat	\
	dc.cat \
	dcda.cat \
	dcdrom.cat \
	dctrl.cat \
	dd.cat \
	ddca.cat \
	ddials.cat \
	defif.cat \
	definet.cat \
	defragfs.cat \
	del.cat	\
	denet.cat \
	deroff.cat \
	devices.cat \
	devices_ent.cat	\
	devices_p.cat \
	devices_tok.cat	\
	devinstall.cat \
	devnm.cat \
	df.cat \
	dfd.cat	\
	dfddi.cat \
	dfsck.cat \
	dgioa.cat \
	dh2.cat	\
	dhf.cat	\
	dhfd.cat \
	dhflvm.cat \
	diagcd.cat \
	diagrpt.cat \
	diags.cat \
	diff.cat \
	diff3.cat \
	diffmk.cat \
	digest.cat \
	diop.cat \
	dircmp.cat \
	dirname.cat \
	disabl.cat \
	diskusg.cat \
	dkbd.cat \
	dkbd_a.cat \
	dlpfk.cat \
	dmpa.cat \
	dmultp.cat \
	dnep.cat \
	dosdir.cat \
	dpm.cat	\
	dpp.cat	\
	dprog.cat \
	druby.cat \
	dsaleth.cat \
	dscreen.cat \
	dscsia.cat \
	dsga.cat \
	dsky.cat \
	dskym.cat \
	dstileth.cat \
	dtablet.cat \
	dtappintegrate.cat \
	dtok32.cat \
	dtoken.cat \
	dtsmit.cat \
	du.cat \
	dump.cat \
	dumpfs.cat \
	dumpi.cat \
	dutil.cat \
	dwbnword.cat \
	dwbpp.cat \
	dwga.cat \
	dx25.cat \
	dxm2p.cat \
	ed.cat \
	edquota.cat \
	enable.cat \
	enq.cat	\
	entstat.cat \
	env.cat	\
	ex.cat \
	execerr.cat \
	expand.cat \
	explain.cat \
	expr.cat \
	factor.cat \
	fddi.cat \
	fddistat.cat \
	feprom_update.cat \
	ff.cat \
	file.cat \
	find.cat \
	finger.cat \
	fingerd.cat \
	fixascii.cat \
	flcopy.cat \
	fold.cat \
	format.cat \
	fpp.cat	\
	fpr.cat	\
	from.cat \
	fsck.cat \
	fsdb.cat \
	fshelp.cat \
	fsplit.cat \
	ftp.cat	\
	ftpd.cat \
	fuser.cat \
	gated.cat \
	genxlt.cat \
	getconf.cat \
	getopt.cat \
	gettable.cat \
	ghost.cat \
	gprof.cat \
	greek.cat \
	grep.cat \
	groups.cat \
	grpck.cat \
	h.ad.cat \
	h.admin.cat \
	h.bd.cat \
	h.bdiff.cat \
	h.cb.cat \
	h.cdc.cat \
	h.cm.cat \
	h.co.cat \
	h.comb.cat \
	h.de.cat \
	h.default.cat \
	h.delta.cat \
	h.ge.cat \
	h.get.cat \
	h.he.cat \
	h.help.cat \
	h.prs.cat \
	h.prskwd.cat \
	h.rc.cat \
	h.rmdel.cat \
	h.sccsdf.cat \
	h.un.cat \
	h.ut.cat \
	h.val.cat \
	h.vc.cat \
	h.what.cat \
	halt.cat \
	hdict.cat \
	He.cmds.cat \
	He.howtos.cat \
	head.cat \
	Hedprf.hdq.cat \
	help.cat \
	helpmux.cat \
	Hemenu.cat \
	Hkeys.map.cat \
	Hprthelp.cat \
	host.cat \
	hostent.cat \
	hostid.cat \
	hostname.cat \
	hplj.cat \
	htable.cat \
	hyphen.cat \
	ibm3816.cat \
	ibm5584.cat \
	ibm5585H-T.cat \
	ibm5587G.cat \
	iconv.cat \
	id.cat \
	ifconfig.cat \
	ifor_common.cat	\
	indent.cat \
	infocmp.cat \
	INeditor.cat \
	INeditorprf.cat	\
	inetd.cat \
	inetexp.cat \
	inetimp.cat \
	inetserv.cat \
	INhistory.cat \
	INindex.cat \
	init.cat \
	INprint.cat \
	install.cat \
	installbsd.cat \
	inuumsg.cat \
	iostat.cat \
	ip_auditpr.cat \
	ipcrm.cat \
	ipcs.cat \
	ipreport.cat \
	iptrace.cat \
	istat.cat \
	jfmtrs.cat \
	join.cat \
	kazfor.cat \
	keycfg.cat \
	keycomp.cat \
	kill.cat \
	killall.cat \
	ksh.cat	\
	last.cat \
	lastcomm.cat \
	ld.cat \
	learn.cat \
	leave.cat \
	lega2.cat \
	lex.cat	\
	lftcmds.cat \
	li.cat \
	libbsd.cat \
	libc.cat \
	libcurses.cat \
	libdbm.cat \
	libffdb.cat \
	libodm.cat \
	libpp.cat \
	libq.cat \
	libqb.cat \
	libs.cat \
	libtcp_ocs.cat \
	libtli.cat \
	license.cat \
	link.cat \
	lint.cat \
	ln.cat \
	locale.cat \
	localedef.cat \
	lock.cat \
	logger.cat \
	logname.cat \
	logout.cat \
	look.cat \
	lorder.cat \
	lp.cat \
	lpd.cat	\
	lpq.cat	\
	lpr.cat	\
	lprm.cat \
	lpstat.cat \
	ls.cat \
	lsadpnm.cat \
	lscfg.cat \
	lsfs.cat \
	lsnamsv.cat \
	lsprtsv.cat \
	lsslip.cat \
	lsusrprof.cat \
	lsvfs.cat \
	m4.cat \
	macref.cat \
	macros.cat \
	magic.cat \
	mail.cat \
	mailstat.cat \
	make.cat \
	makedev.cat \
	man.cat	\
	memory.cat \
	mesg.cat \
	mh.cat \
	mirrord.cat \
	mkboot.cat \
	mkdir.cat \
	mkfifo.cat \
	mkfs.cat \
	mkhosts.cat \
	mkitab.cat \
	mknamsv.cat \
	mkprtsv.cat \
	mknod.cat \
	mkproto.cat \
	mkslip.cat \
	mkstr.cat \
	mksysb.cat \
	mkusrprof.cat \
	mm.cat \
	monitord_admin.cat \
	more.cat \
	morps.cat \
	mount.cat \
	mouse.cat \
	mouse_a.cat \
	mpa.cat	\
	mpcfg.cat \
	msgfac.cat \
	msgs.cat \
	mv.cat \
	mvdir.cat \
	named.cat \
	namerslv.cat \
	ncheck.cat \
	ncs.cat	\
	nddstat.cat \
	ndx.cat	\
	netls.cat \
	netstat.cat \
	newfile.cat \
	newform.cat \
	news.cat \
	nice.cat \
	nl.cat \
	nm.cat \
	no.cat \
	nohup.cat \
	notinet.cat \
	nslookup.cat \
	ntsc.cat \
	nwords.cat \
	od.cat \
	odmcmd.cat \
	op_check.cat \
	op_extend.cat \
	op_make.cat \
	op_namei.cat \
	optics.cat \
	pac.cat	\
	pack.cat \
	panel20.cat \
	paste.cat \
	pathchk.cat \
	patch.cat \
	pax.cat	\
	pcat.cat \
	pcmciastat.cat \
	ped.cat	\
	penable.cat \
	pg.cat \
	ping.cat \
	pioattr1.cat \
	pioattr2.cat \
	piobe.cat \
	piosmit.cat \
	piosplp.cat \
	plotgbe.cat \
	pmctrl.cat \
	poundfile.cat \
	pr.cat \
	printf.cat \
	printinfo.cat \
	prof.cat \
	proto.cat \
	prtty.cat \
	ps.cat \
	ptx.cat	\
	pw.cat \
	pwdck.cat \
	qadm.cat \
	qcadm.cat \
	qcan.cat \
	qchk.cat \
	qhld.cat \
	qmain.cat \
	qmov.cat \
	qpri.cat \
	qprt.cat \
	qstat.cat \
	quot.cat \
	quota.cat \
	quotacheck.cat \
	quotaon.cat \
	ranlib.cat \
	rbyname.cat \
	rc.cat \
	rc_powerfail.cat \
	rcp.cat	\
	rcs.cat	\
	rdist.cat \
	readfile.cat \
	reboot.cat \
	refer.cat \
	regcmp.cat \
	rem.cat	\
	renice.cat \
	repquota.cat \
	restore.cat \
	restore2.cat \
	rev.cat	\
	rexec.cat \
	rexecd.cat \
	rlogin.cat \
	rlogind.cat \
	rm.cat \
	rmail.cat \
	rmhist.cat \
	rmnamsv.cat \
	rmprtsv.cat \
	rmslip.cat \
	rmt.cat	\
	rmusrprof.cat \
	roff.cat \
	route.cat \
	routed.cat \
	rpl.cat	\
	rsh.cat	\
	rshd.cat \
	ruptime.cat \
	ruser.cat \
	rwho.cat \
	rwhod.cat \
	sa.cat \
	sadc.cat \
	sar.cat	\
	savecr.cat \
	sccs.cat \
	scdisk.cat \
	scls.cat \
	script.cat \
	sdiff.cat \
	sed.cat	\
	semutil.cat \
	sendmail.cat \
	setclock.cat \
	setmaps.cat \
	setupterm.cat \
	shell.cat \
	showrem.cat \
	showstring.cat \
	shutdown.cat \
	size.cat \
	skulker.cat \
	slattach.cat \
	sleep.cat \
	slipcomm.cat \
	sliplogin.cat \
	slipserver.cat \
	sm_cmdbsys.cat \
	smit.cat \
	snmpd.cat \
	sno.cat	\
	sort.cat \
	spell.cat \
	split.cat \
	src.cat	\
	srvboot.cat \
	stinet.cat \
	stpinet.cat \
	strace.cat \
	strchg.cat \
	strclean.cat \
	strconf.cat \
	strerr.cat \
	strinfo.cat \
	strings.cat \
	strip.cat \
	strload.cat \
	struct.cat \
	sttinet.cat \
	stty-cxma.cat \
	stty-lion.cat \
	stty-rs.cat \
	stty.cat \
	style.cat \
	su.cat \
	survd.cat \
	sum.cat	\
	swvpd.cat \
	syncd.cat \
	sys.cat	\
	syscall.cat \
	sysck.cat \
	sysline.cat \
	syslogd.cat \
	sysx.cat \
	tab.cat	\
	tab_a.cat \
	tabs.cat \
	tail.cat \
	talk.cat \
	talkd.cat \
	tar.cat	\
	tc.cat \
	tcbauth.cat \
	tcbck.cat \
	tcopy.cat \
	tcpcfglb.cat \
	tcpdump.cat \
	tctl.cat \
	tdigest.cat \
	tee.cat	\
	telnet.cat \
	telnetd.cat \
	termc.cat \
	termdef.cat \
	test.cat \
	tftp.cat \
	tftpd.cat \
	tic.cat	\
	time.cat \
	timed.cat \
	tip.cat	\
	tokstat.cat \
	touch.cat \
	tput.cat \
	tr.cat \
	traceroute.cat \
	trbsd.cat \
	trpt.cat \
	tset.cat \
	tsm.cat	\
	tsort.cat \
	tty.cat	\
	u5081.cat \
	ubackrest.cat \
	ubump.cat \
	ucfgdev.cat \
	ucfgif.cat \
	ucfginet.cat \
	ucfgvpd.cat \
	uconvdef.cat \
	udefif.cat \
	udefinet.cat \
	udiagmon.cat \
	udiagup.cat \
	udiskmnt.cat \
	udmedit.cat \
	udsense.cat \
	udtl.cat \
	uenet.cat \
	ufd.cat	\
	ufmt.cat \
	ugenucodesa.cat \
	uhint.cat \
	ukey.cat \
	ul.cat \
	ulan.cat \
	umblist.cat \
	umcode.cat \
	umkdskt.cat \
	umlc.cat \
	ump.cat	\
	uname.cat \
	unifdef.cat \
	uniq.cat \
	units.cat \
	unix.cat \
	unpack.cat \
	uscsi.cat \
	users.cat \
	usrck.cat \
	uucp.cat \
	uudecode.cat \
	uuencode.cat \
	uusend.cat \
	vac.cat	\
	versions.cat \
	virscan.cat \
	vgrind.cat \
	vmstat.cat \
	w.cat \
	wall.cat \
	watch.cat \
	wc.cat \
	whereis.cat \
	which.cat \
	who.cat	\
	whois.cat \
	whoami.cat \
	write.cat \
	writesrv.cat \
	xargs.cat \
	xpreview.cat \
	xsend.cat \
	xshell.cat \
	xstr.cat \
	yacc.cat \
	yacc_user.cat

SCCS_MSG_LIST	= \
	sccsdiff.msg \
	cm.msg \
	co.msg \
	admin.msg \
	comb.msg \
	delta.msg \
	get.msg	\
	sccshelp.msg \
	prs.msg	\
	rmchg.msg \
	unget.msg \
	val.msg	\
	vc.msg \
	bsd.msg

SMIT_MSG_LIST	= \
	sm_inst.msg \
	sm_tcbauth.msg \
	tcpip.msg \
	dlccfg.msg \
	sm_lft.msg \
	sm_cmderrlg.msg	 \
	sm_cmdtrace.msg	\
	sm_cmddump.msg \
	cmdlvmsmit.msg \
	cmdstat.msg  \
	cmdque.msg \
	cmdfs.msg \
	cmdcntl.msg \
	cmdarch.msg \
	cmdoper.msg \
	cmdcomm.msg \
	x11.msg	\
	menu.25.msg  \
	cfgmeth.msg \
	sm_cmdps.msg \
	cmdlice.msg \
	cmdtz.msg  \
	sm_cmdnfs.msg \
	cmdsrc.msg \
	cmdiconv.msg \
	cmdmsg.msg \
	sm_rds.msg \
	sm_cmdalog.msg \
	sm_netls.msg \
	sm_assist.msg \
	sm_mle.msg \
	sm_netls2.msg \
	pwrcmd.msg \
	smit.98.msg \
	smit.99.msg  \
	smit.101.msg

UNIX_MSG_LIST =	\
	unixset.msg \
	syspfs.msg

CODEPOINT_LIST	= \
	codepoint_setD.mri \
	codepoint_setE.mri \
	codepoint_setF.mri \
	codepoint_setI.mri \
	codepoint_setP.mri \
	codepoint_setR.mri \
	codepoint_setU.mri

HEDPRF_MSG_LIST	= \
	edprf.epii

NCS_MSG_LIST = \
	lsl.msg	\
	lss.msg	\
	nck.msg	\
	perf.msg \
	cps.msg	\
	glb.msg	\
	example.msg

OTHERS	= \
	codepoint.cat

ILIST	= ${OTHERS}
IDIR	= ${MSGLANGPATH}/

.include <${RULES_MK}>

.if defined(SOURCE_CODESET) && defined(TARGET_CODESET)
codepoint.mri:	${CODEPOINT_LIST}
	${CAT} ${.ALLSRC} | ${ICONV} -f	${SOURCE_CODESET} -t ${TARGET_CODESET} > ${.TARGET}
.else
codepoint.mri:	${CODEPOINT_LIST}
	${CAT} ${.ALLSRC} > ${.TARGET}
.endif

codepoint.cat:	codepoint.mri
	${RM} -f ${.TARGET}
	${ERRPREFIX} ${codepoint.mri:P}	- | LOCPATH=${LOCPATH} LC_CTYPE=${MSGLANG} ${ERRINST} -q -z ${.TARGET}

sccs.msg:	${SCCS_MSG_LIST}
	${CAT} ${.ALLSRC} > ${.TARGET}

smit.msg:	${SMIT_MSG_LIST}
	${CAT} ${.ALLSRC} > ${.TARGET}

unix.msg:	${UNIX_MSG_LIST}
	${CAT} ${.ALLSRC} > ${.TARGET}

ncs.msg:	${NCS_MSG_LIST}
	${CAT} ${.ALLSRC} > ${.TARGET}

Hedprf.hdq.msg:	${HEDPRF_MSG_LIST}
	${AWK} 'BEGIN {	line=0 ; printf	"\n$$quote \"\n$$set 1\n\n" ; } \
	    ( $$0 !~ "^#" ) { printf "%d	\"%s\"\n", ++line, $$0 ; } \
	    END	{ while( line++	< 1000 ) { printf "%d \"\"\n", line ; }	}' ${.ALLSRC} >	${.TARGET}
