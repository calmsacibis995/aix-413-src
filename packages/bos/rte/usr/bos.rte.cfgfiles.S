# @(#)81	1.25  src/packages/bos/rte/usr/bos.rte.cfgfiles.S, pkgbos, pkg41J, 9518A_all 5/2/95 08:54:11
#
#   COMPONENT_NAME: pkgbos
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
/etc/csh.cshrc			preserve
/etc/csh.login			preserve
/etc/consdef			v4preserve
/etc/dlpi.conf			v4preserve
/etc/dumpdates			preserve
/etc/environment		preserve
/etc/filesystems		preserve
/etc/group			preserve
/etc/inittab			v4preserve
/etc/magic			v4preserve
/etc/motd			user_merge
/etc/objrepos/CDiagDev		v4preserve
/etc/objrepos/Config_Rules	v4preserve
/etc/objrepos/CuAt		other
/etc/objrepos/CuAt.vc		other
/etc/objrepos/CuDep		other
/etc/objrepos/CuDv		other
/etc/objrepos/CuDvDr		other
/etc/objrepos/CuVPD		other
/etc/objrepos/DAVars		v4preserve
/etc/objrepos/FRUB		v4preserve
/etc/objrepos/FRUs		v4preserve
/etc/objrepos/MenuGoal		v4preserve
/etc/objrepos/SRCnotify		preserve
/etc/objrepos/SRCsubsvr		preserve
/etc/objrepos/SRCsubsys		preserve
/etc/objrepos/SWservAt 		preserve
/etc/objrepos/SWservAt.vc	preserve
/etc/objrepos/TMInput		v4preserve
/etc/profile			preserve
/etc/pse.conf			v4preserve
/etc/qconfig			preserve
/etc/rc				user_merge
/etc/security/audit/config	preserve
/etc/security/audit/objects	preserve
/etc/security/environ		preserve
/etc/security/group		preserve
/etc/security/lastlog		preserve
/etc/security/passwd		preserve
/etc/security/.profile		preserve
/etc/swapspaces			preserve
/etc/trcfmt			preserve
/etc/uucp/Devices		preserve
/etc/uucp/Dialers		preserve
/etc/tsh_profile		user_merge
/etc/xtiso.conf			v4preserve
/sbin/rc.boot			user_merge
/usr/lib/objrepos/DSMenu	v4preserve
/usr/lib/objrepos/DSMOptions	v4preserve
/usr/lib/objrepos/DSMOptions.vc	v4preserve
/usr/lib/objrepos/GAI		v4preserve
/usr/lib/objrepos/GAI.vc	v4preserve
/usr/lib/objrepos/PdAt		v4preserve
/usr/lib/objrepos/PdAt.vc	v4preserve
/usr/lib/objrepos/PdCn		v4preserve
/usr/lib/objrepos/PdDv		v4preserve
/usr/lib/objrepos/PdDv.vc	v4preserve
/usr/lib/objrepos/XINPUT	v4preserve
/usr/lib/objrepos/XINPUT.vc	v4preserve
/usr/lib/objrepos/fix		user_merge
/usr/lib/objrepos/fix.vc	user_merge
/var/adm/cron/at.deny			v4preserve
/var/adm/cron/log			v4preserve
/var/adm/ras/codepoint.cat		preserve
/var/adm/ras/errtmplt			preserve
/var/spool/cron/crontabs/adm		v4preserve
/var/spool/cron/crontabs/root		v4preserve
/var/spool/cron/crontabs/sys		v4preserve
# Merge method files put at end in case they use temporary space in /tmp
/etc/objrepos/errnotify		other	   /usr/lpp/bos/migrate/convert_errnotify
/etc/passwd			auto_merge	/usr/lpp/bos/migrate/passwd_mig
# login_mrg must run before user_mrg.
/etc/security/login.cfg		auto_merge	/usr/lpp/bos/migrate/login_mrg
/etc/security/user		auto_merge	/usr/lpp/bos/migrate/user_mrg
# The following 4 files are merged by secur_mig.
/etc/security/limits		auto_merge	/usr/lpp/bos/migrate/secur_mig
/etc/security/audit/bincmds	auto_merge
/etc/security/audit/events	auto_merge
/etc/security/audit/streamcmds	auto_merge
/usr/lib/security/mkuser.default	auto_merge	/usr/lpp/bos/migrate/mkusr_mig
/var/adm/ras/errlog			other	/usr/lpp/bos/migrate/convert_errlog -i /tmp/bos/var/adm/ras/errlog
/usr/lib/objrepos/sm_cmd_hdr		other		/usr/lpp/bos/migrate/merge_smit_db /tmp/bos/usr/lib/objrepos /usr/lib/objrepos
/usr/lib/objrepos/sm_cmd_hdr.vc		other
/usr/lib/objrepos/sm_cmd_opt		other
/usr/lib/objrepos/sm_cmd_opt.vc		other
/usr/lib/objrepos/sm_menu_opt		other
/usr/lib/objrepos/sm_menu_opt.vc	other
/usr/lib/objrepos/sm_name_hdr		other
/usr/lib/objrepos/sm_name_hdr.vc	other
/etc/objrepos/history		other       /usr/lpp/bos/migrate/merge_swvpds
/etc/objrepos/history.vc	other
/etc/objrepos/inventory		other
/etc/objrepos/inventory.vc	other
/etc/objrepos/lpp		other
/etc/objrepos/lpp.vc		other
/etc/objrepos/product		other
/etc/objrepos/product.vc	other
/usr/lib/objrepos/history	other
/usr/lib/objrepos/history.vc	other
/usr/lib/objrepos/inventory	other
/usr/lib/objrepos/inventory.vc	other
/usr/lib/objrepos/lpp		other
/usr/lib/objrepos/lpp.vc	other
/usr/lib/objrepos/product	other
/usr/lib/objrepos/product.vc	other
/usr/share/lib/objrepos/history		other
/usr/share/lib/objrepos/history.vc	other
/usr/share/lib/objrepos/inventory	other
/usr/share/lib/objrepos/inventory.vc	other
/usr/share/lib/objrepos/lpp		other
/usr/share/lib/objrepos/lpp.vc		other
/usr/share/lib/objrepos/product		other
/usr/share/lib/objrepos/product.vc	other
/etc/security/sysck.cfg		user_merge /usr/lpp/bos/migrate/SysckMerge /tmp/bos/etc/security/sysck.cfg /etc/security/sysck.cfg
