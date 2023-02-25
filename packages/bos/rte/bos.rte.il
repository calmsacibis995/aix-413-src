# @(#)60	1.252  src/packages/bos/rte/bos.rte.il, pkgbos, pkg41J, 9522A_all 5/30/95 18:12:21
#
# COMPONENT_NAME: pkgbos
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.rte
#
# The following /usr directories MUST be the first in the list so that
# the BOSINSTL and adeinventory commands will work correctly.
#
D 2 2 755	/ rte
d 2 2 755	/usr rte
D 2 2 755	/usr/lpp rte
D 2 2 755	/usr/lpp/bos rte
D 2 2 755	/usr/lpp/bos/inst_root rte
D 2 2 755	/usr/include rte
D 2 2 755	/usr/include/sys rte
D 2 2 755	/usr/lib rte
s 2 2 777	/lib /usr/lib rte
D 2 2 755       /usr/lib/threads rte
D 2 2 775	/usr/ucb rte
D 2 2 775	/usr/usg rte
#
d 0 0 775	/dev rte
d 0 0 755	/etc rte
v 0 0 644	/etc/consdef devices
v 2 2 555	/etc/csh.cshrc shell
v 2 2 555	/etc/csh.login shell
v 2 2 644	/etc/dlpi.conf tty 
v 2 0 664	/etc/dumpdates serv_aid
v 0 0 664	/etc/environment control
v 0 0 664	/etc/filesystems filesystem
vt 0 7 664	/etc/group security
v 0 0 644	/etc/inittab control
v 2 2 555	/etc/magic commands
v 2 2 544	/etc/motd security
d 0 0 775	/etc/objrepos rte
v 0 0 644	/etc/objrepos/CDiagDev diag
v 0 0 644	/etc/objrepos/Config_Rules devices
v 0 0 644	/etc/objrepos/CuAt devices
v 0 0 644	/etc/objrepos/CuAt.vc devices
v 0 0 644	/etc/objrepos/CuDep devices
v 0 0 644	/etc/objrepos/CuDv devices
v 0 0 644	/etc/objrepos/CuDvDr devices
v 0 0 644	/etc/objrepos/CuVPD devices
v 0 0 644	/etc/objrepos/DAVars diag
v 0 0 644	/etc/objrepos/FRUB diag
v 0 0 644	/etc/objrepos/FRUs diag
v 0 0 644	/etc/objrepos/MenuGoal diag
v 0 0 664	/etc/objrepos/SRCnotify SRC
v 0 0 664	/etc/objrepos/SRCsubsvr SRC
v 0 0 664	/etc/objrepos/SRCsubsys SRC
v 0 0 664	/etc/objrepos/SWservAt serv_aid
v 0 0 664	/etc/objrepos/SWservAt.vc serv_aid
v 0 0 644	/etc/objrepos/TMInput diag
v 0 0 664	/etc/objrepos/errnotify serv_aid
v 0 0 664	/etc/objrepos/history rte
v 0 0 664	/etc/objrepos/history.vc rte
v 0 0 664	/etc/objrepos/inventory rte
v 0 0 664	/etc/objrepos/inventory.vc rte
v 0 0 664	/etc/objrepos/lpp rte
v 0 0 664	/etc/objrepos/lpp.vc rte
v 0 0 664	/etc/objrepos/product rte
v 0 0 664	/etc/objrepos/product.vc rte
vt 0 7 664	/etc/passwd security
v 2 2 555	/etc/profile shell
v 2 2 644	/etc/pse.conf tty
vt 0 9 664	/etc/qconfig printers
v 2 2 554	/etc/rc boot
v 0 0 700	/etc/rc.powerfail control
d 0 7 750	/etc/security rte
d 0 10 750	/etc/security/audit rte
v 0 10 640	/etc/security/audit/bincmds security
v 0 10 640	/etc/security/audit/config security
v 0 10 640	/etc/security/audit/events security
v 0 10 640	/etc/security/audit/objects security
v 0 10 640	/etc/security/audit/streamcmds security
v 0 7 640	/etc/security/environ security
vt 0 7 640	/etc/security/group security
vt 0 7 640	/etc/security/lastlog security
v 0 7 640	/etc/security/limits security
v 0 7 660	/etc/security/login.cfg security
vt 0 7 600	/etc/security/passwd security
vt 0 7 640	/etc/security/portlog security
v 0 7 660	/etc/security/.profile security
v 0 7 640	/etc/security/sysck.cfg security
vt 0 7 640	/etc/security/user security
v 0 0 664	/etc/swapspaces boot
a 2 2 644	/etc/trcfmt.Z serv_aid
vt 0 0 444	/etc/tsh_profile security
d 0 3 755       /etc/uucp tty
D 0 3 755       /usr/lib/uucp tty
v 5 5 444       /etc/uucp/Devices tty
S 5 5 777       /usr/lib/uucp/Devices /etc/uucp/Devices tty
v 5 5 444       /etc/uucp/Dialers tty
S 5 5 777       /usr/lib/uucp/Dialers /etc/uucp/Dialers tty
v 0 0 664	/etc/vfs filesystem
d 0 0 775	/etc/vg rte
v 2 2 644	/etc/xtiso.conf tty 
d 2 2 755	/home rte
s 2 2 777	/u /home rte
d 100 100 755	/home/guest rte
d 0 0 700	/home/lost+found rte
d 0 0 700	/lost+found rte
d 2 2 755	/lpp rte
d 2 2 755	/mnt rte
d 0 0 755	/opt rte
d 2 2 755	/sbin rte
f 0 0 555	/sbin/comp.kext jfscomp
f 0 0 555	/sbin/comp.uext jfscomp
v 0 0 774	/sbin/rc.boot boot
d 0 0 775	/sbin/helpers rte
f 2 2 555	/sbin/helpers/v3fshelper filesystem
d 2 2 1777	/tmp rte
d 0 0 700	/tmp/lost+found rte
D 2 2 755	/usr/bin rte
s 2 2 777	/bin /usr/bin rte
FT 2 2 555	/usr/bin/acledit security
FT 2 2 555	/usr/bin/aclget security
FT 2 2 555	/usr/bin/aclput security
F 2 2 555	/usr/bin/alias shell
H 2 2 555	/usr/bin/bg /usr/bin/alias shell
H 2 2 555	/usr/bin/cd /usr/bin/alias shell
H 2 2 555	/usr/bin/command /usr/bin/alias shell
H 2 2 555	/usr/bin/fc /usr/bin/alias shell
H 2 2 555	/usr/bin/fg /usr/bin/alias shell
H 2 2 555	/usr/bin/getopts /usr/bin/alias shell
H 2 2 555	/usr/bin/hash /usr/bin/alias shell
H 2 2 555	/usr/bin/jobs /usr/bin/alias shell
H 2 2 555	/usr/bin/read /usr/bin/alias shell
H 2 2 555	/usr/bin/type /usr/bin/alias shell
H 2 2 555	/usr/bin/ulimit /usr/bin/alias shell
H 2 2 555	/usr/bin/umask /usr/bin/alias shell
H 2 2 555	/usr/bin/unalias /usr/bin/alias shell
H 2 2 555	/usr/bin/wait /usr/bin/alias shell
F 2 2 555	/usr/bin/alog serv_aid
F 2 2 555	/usr/bin/apply shell
FT 0 8 6555	/usr/bin/at cron
FT 2 8 2555	/usr/bin/atq cron
FT 2 2 555	/usr/bin/atrm cron
F 2 2 555	/usr/bin/awk shell
H 2 2 555	/usr/bin/nawk /usr/bin/awk shell
F 2 2 555	/usr/bin/banner misc_cmds
F 2 2 555	/usr/bin/basename shell
F 2 2 555	/usr/bin/batch cron
F 2 2 555	/usr/bin/bc misc_cmds
F 2 2 555	/usr/bin/bdiff compare
F 2 2 555	/usr/bin/bfs commands
F 0 0 550	/usr/bin/bootlist boot
F 2 2 555	/usr/bin/bsh shell
H 2 2 555	/usr/bin/Rsh /usr/bin/bsh shell
F 2 2 555	/usr/bin/cal misc_cmds
F 2 2 555	/usr/bin/calendar misc_cmds
F 2 2 555	/usr/bin/cancel printers
F 0 0 4555	/usr/bin/capture shell
F 2 2 555	/usr/bin/cat commands
FT 0 7 2555	/usr/bin/chfn security
FT 0 7 4550	/usr/bin/chgroup security
F 2 2 555	/usr/bin/chgrp security
FT 0 7 2555	/usr/bin/chgrpmem security
F 2 2 555	/usr/bin/chlang ILS
F 2 2 555	/usr/bin/chmod commands
F 0 2 555	/usr/bin/chown commands
FT 0 9 6550	/usr/bin/chque printers
FT 0 9 6550	/usr/bin/chquedev printers
FT 0 7 4550	/usr/bin/chsec security
F 0 0 550	/usr/bin/chserver SRC
FT 0 7 2555	/usr/bin/chsh security
F 0 0 550	/usr/bin/chssys SRC
F 2 2 555	/usr/bin/chtz date
FT 0 7 4550	/usr/bin/chuser security
F 2 2 555	/usr/bin/cksum commands
F 2 2 555	/usr/bin/clear tty
F 2 2 555	/usr/bin/cmp compare
F 2 2 555	/usr/bin/colrm edit
F 2 2 555	/usr/bin/comm edit
F 2 2 555	/usr/bin/compress archive
H 2 2 555	/usr/bin/uncompress /usr/bin/compress archive
H 2 2 555	/usr/bin/zcat /usr/bin/compress archive
F 2 2 550	/usr/bin/copydump serv_aid
F 2 2 550	/usr/bin/copydumpmenu serv_aid
V 2 2 555	/usr/bin/cp commands
F 2 2 555	/usr/bin/cpio archive
FT 0 8 2700	/usr/bin/cronadm cron
FT 0 8 6555	/usr/bin/crontab cron
F 2 2 555	/usr/bin/csh shell
F 2 2 555	/usr/bin/csplit edit
F 2 2 555	/usr/bin/ctags edit
F 2 2 555	/usr/bin/cut edit
F 2 2 555	/usr/bin/date date
F 2 2 555	/usr/bin/dc misc_cmds
F 2 2 555	/usr/bin/dd archive
FT 2 2 555	/usr/bin/df filesystem
F 2 2 555	/usr/bin/diff compare
F 2 2 555	/usr/bin/diff3 compare
F 2 2 555	/usr/bin/dircmp compare
F 2 2 555	/usr/bin/dirname shell
F 2 2 555	/usr/bin/dspcat ILS
F 2 2 555	/usr/bin/dspmsg ILS
F 2 2 555	/usr/bin/du commands
F 2 2 555	/usr/bin/echo shell
F 2 2 555	/usr/bin/ed edit
H 2 2 555	/usr/bin/red /usr/bin/ed edit
FT 0 9 6555	/usr/bin/enq printers
F 2 2 555	/usr/bin/env shell
H 2 2 555	/usr/bin/printenv /usr/bin/env shell
F 0 2 544	/usr/bin/errinstall serv_aid
F 0 2 544	/usr/bin/errupdate serv_aid
F 2 2 555	/usr/bin/ex edit
H 2 2 555	/usr/bin/edit /usr/bin/ex edit
H 2 2 555	/usr/bin/vedit /usr/bin/ex edit
H 2 2 555	/usr/bin/vi /usr/bin/ex edit
H 2 2 555	/usr/bin/view /usr/bin/ex edit
F 2 2 555	/usr/bin/expand edit
F 2 2 555	/usr/bin/expr shell
F 2 2 555	/usr/bin/factor misc_cmds
F 2 2 555	/usr/bin/false shell
F 2 2 555	/usr/bin/file commands
F 2 2 555	/usr/bin/find commands
F 2 2 555	/usr/bin/fold edit
F 2 2 555	/usr/bin/gencat ILS
F 2 2 555	/usr/bin/getconf shell
F 2 2 555	/usr/bin/getopt commands
F 2 2 555	/usr/bin/grep commands
H 2 2 555	/usr/bin/fgrep /usr/bin/grep commands
H 2 2 555	/usr/bin/egrep /usr/bin/grep commands
FT 2 2 555	/usr/bin/groups security
F 2 2 555	/usr/bin/head commands
F 2 2 555	/usr/bin/help man
F 2 2 555	/usr/bin/hostname net
FT 2 2 555	/usr/bin/id security
F 0 0 555	/usr/bin/install commands
F 0 0 555	/usr/bin/installbsd commands
H 0 0 555	/usr/ucb/install /usr/bin/installbsd commands
FT 2 2 555	/usr/bin/ipcrm control
FT 0 2 4555	/usr/bin/ipcs control
FT 2 2 555	/usr/bin/istat filesystem
F 2 2 555	/usr/bin/join edit
F 2 2 555	/usr/bin/kill control
FT 2 2 555	/usr/bin/ksh shell
HT 2 2 555	/usr/bin/sh /usr/bin/ksh shell
HT 2 2 555	/usr/bin/psh /usr/bin/ksh shell
HT 2 2 555	/usr/bin/tsh /usr/bin/ksh shell
F 2 2 555	/usr/bin/last misc_cmds
F 2 2 555	/usr/bin/lastcomm misc_cmds
F 2 2 555	/usr/bin/leave date
F 2 2 555	/usr/bin/line shell
V 2 2 555	/usr/bin/ln commands
F 2 2 555	/usr/bin/locale ILS
F 2 2 555	/usr/bin/lock security
F 2 2 555	/usr/bin/logger serv_aid
FT 2 2 555	/usr/bin/logname security
FT 0 0 4555	/usr/bin/logout security
F 2 2 555	/usr/bin/look compare
F 2 2 555	/usr/bin/lp printers
F 2 2 555	/usr/bin/lppchk install
F 2 2 555	/usr/bin/lpq printers
F 2 2 555	/usr/bin/lpr printers
F 2 2 555	/usr/bin/lprm printers
F 2 2 555	/usr/bin/lpstat printers
F 2 2 555	/usr/bin/ls commands
F 2 2 555	/usr/bin/lsallq printers
F 2 2 555	/usr/bin/lsallqdev printers
F 2 2 555	/usr/bin/lslpp install
F 2 2 555	/usr/bin/lsque printers
F 2 2 555	/usr/bin/lsquedev printers
FT 0 7 4550	/usr/bin/lssec security
FT 0 0 2555	/usr/bin/lssrc SRC
F 2 2 555	/usr/bin/man man
H 2 2 555	/usr/bin/apropos /usr/bin/man man
H 2 2 555	/usr/bin/whatis /usr/bin/man man
F 0 2 4555	/usr/bin/mesg misc_cmds
F 2 2 555	/usr/bin/mkdir commands
F 2 2 555	/usr/bin/mkfifo shell
FT 0 7 4550	/usr/bin/mkgroup security
F 0 0 550	/usr/bin/mknotify SRC
FT 0 9 6550	/usr/bin/mkque printers
FT 0 9 6550	/usr/bin/mkquedev printers
F 0 0 550	/usr/bin/mkserver SRC
F 0 0 550	/usr/bin/mkssys SRC
FT 0 7 4550	/usr/bin/mkuser security
F 2 2 555	/usr/bin/more commands
V 2 2 555	/usr/bin/mv commands
F 2 2 555	/usr/bin/newform edit
FT 0 7 4555	/usr/bin/newgrp security
F 2 2 555	/usr/bin/news security
F 2 2 555	/usr/bin/nice control
F 2 2 555	/usr/bin/nl edit
F 2 2 555	/usr/bin/nohup control
FT 2 2 555	/usr/bin/od edit
F 2 2 555	/usr/bin/odmadd odm
F 2 2 555	/usr/bin/odmchange odm
F 2 2 555	/usr/bin/odmcreate odm
F 2 2 555	/usr/bin/odmdelete odm
F 2 2 555	/usr/bin/odmdrop odm
F 2 2 555	/usr/bin/odmget odm
F 2 2 555	/usr/bin/odmshow odm
F 2 2 555	/usr/bin/oslevel install
F 2 2 555	/usr/bin/pack archive
F 2 2 555	/usr/bin/pagesize misc_cmds
FT 0 7 4555	/usr/bin/passwd security
F 2 2 555	/usr/bin/paste edit
F 2 2 555	/usr/bin/patch shell
F 2 2 555	/usr/bin/pathchk shell
F 2 2 555	/usr/bin/pax archive
F 2 2 555	/usr/bin/pcat archive
F 2 2 555	/usr/bin/pg commands
F 2 2 555	/usr/bin/pr commands
F 2 2 555	/usr/bin/printf commands
FT 2 0 2555	/usr/bin/ps control
F 2 2 555	/usr/bin/pwd commands
FT 0 7 4550	/usr/bin/pwdadm security
FT 0 7 4550	/usr/bin/pwdck security
FT 0 0 2550	/usr/bin/refresh SRC
F 2 2 555	/usr/bin/rev edit
V 2 2 555	/usr/bin/rm commands
H 2 2 555	/usr/bin/rmdir /usr/bin/rm commands
F 0 0 550	/usr/bin/rmnotify SRC
FT 0 9 6550	/usr/bin/rmque printers
FT 0 9 6550	/usr/bin/rmquedev printers
F 0 0 550	/usr/bin/rmserver SRC
F 0 0 550	/usr/bin/rmssys SRC
F 0 0 4555	/usr/bin/script shell
F 2 2 555	/usr/bin/sdiff compare
F 2 2 555	/usr/bin/sed edit
FT 0 7 4555	/usr/bin/setgroups security
FT 0 7 4555	/usr/bin/setsenv security
FT 0 7 4555	/usr/bin/shell security
F 2 2 555	/usr/bin/sleep control
F 2 2 555	/usr/bin/sort commands
F 2 2 555	/usr/bin/split edit
FT 0 0 2550	/usr/bin/startsrc SRC
FT 0 0 2550	/usr/bin/stopsrc SRC
F 0 2 755	/usr/bin/stty tty
FT 0 7 4555	/usr/bin/su security
F 2 2 555	/usr/bin/sum commands
F 2 2 555	/usr/bin/syscall shell
FT 0 7 4550	/usr/bin/sysck install
F 0 2 544	/usr/bin/sysdumpdev serv_aid
F 2 2 555	/usr/bin/tab commands
H 2 2 555	/usr/bin/untab /usr/bin/tab commands
F 0 2 555	/usr/bin/tabs tty
F 2 2 555	/usr/bin/tail commands
F 2 2 555	/usr/bin/tar archive
FT 0 7 4550	/usr/bin/tcbck security
F 2 2 555	/usr/bin/tcopy archive
F 2 2 555	/usr/bin/tctl archive
H 2 2 555	/usr/bin/mt /usr/bin/tctl archive
FT 2 2 555	/usr/bin/tee commands
FT 2 2 555	/usr/bin/termdef tty
F 2 2 555	/usr/bin/test shell
F 2 2 555	/usr/bin/time misc_cmds
F 2 2 555	/usr/bin/touch commands
F 0 2 555	/usr/bin/tput tty
F 2 2 555	/usr/bin/tr commands
F 2 2 555	/usr/bin/trbsd commands
H 2 2 555	/usr/ucb/tr /usr/bin/trbsd commands
F 2 2 550	/usr/bin/trcupdate serv_aid
F 2 2 555	/usr/bin/true shell
F 2 2 555	/usr/bin/tset tty
H 2 2 555	/usr/bin/reset /usr/bin/tset tty
F 2 2 555	/usr/bin/tty control
FT 2 2 555	/usr/bin/tvi edit
F 2 2 555	/usr/bin/uname misc_cmds
F 2 2 555	/usr/bin/unexpand edit
F 2 2 555	/usr/bin/uniq commands
F 2 2 555	/usr/bin/units misc_cmds
F 2 2 555	/usr/bin/unpack archive
FT 2 2 555	/usr/bin/users security
HT 2 2 555	/usr/bin/u /usr/bin/users security
FT 0 7 4550	/usr/bin/usrck security
FT 0 0 2555	/usr/bin/w misc_cmds
HT 0 0 2555	/usr/bin/uptime /usr/bin/w misc_cmds
F 2 2 555	/usr/bin/what bind_cmds
S 2 2 777	/usr/ucb/what /usr/bin/what bind_cmds
F 2 2 555	/usr/bin/wc commands
F 2 2 555	/usr/bin/whereis commands
F 2 2 555	/usr/bin/which shell
FT 2 2 555	/usr/bin/who misc_cmds
F 2 2 555	/usr/bin/whoami misc_cmds
F 2 2 555	/usr/bin/write misc_cmds
F 2 2 555	/usr/bin/xargs shell
F 2 2 555	/usr/bin/yes misc_cmds
D 2 2 755	/usr/ccs rte
D 2 2 755	/usr/ccs/bin rte
F 2 2 555	/usr/ccs/bin/ar bind_cmds
S 2 2 777	/usr/bin/ar /usr/ccs/bin/ar bind_cmds
F 2 2 555	/usr/ccs/bin/bind bind_cmds
S 2 2 777	/usr/lib/bind /usr/ccs/bin/bind bind_cmds
F 2 2 555	/usr/ccs/bin/ld bind_cmds
S 2 2 777	/usr/bin/ld /usr/ccs/bin/ld bind_cmds
F 2 2 555	/usr/ccs/bin/lorder bind_cmds
S 2 2 777	/usr/bin/lorder /usr/ccs/bin/lorder bind_cmds
F 2 2 555	/usr/ccs/bin/nm bind_cmds
S 2 2 777	/usr/usg/nm /usr/ccs/bin/nm bind_cmds
S 2 2 777	/usr/bin/nm /usr/ccs/bin/nm bind_cmds
F 2 2 555	/usr/ccs/bin/ranlib bind_cmds
S 2 2 777	/usr/bin/ranlib /usr/ccs/bin/ranlib bind_cmds
FT 2 2 555	/usr/ccs/bin/strings bind_cmds
ST 2 2 777	/usr/bin/strings /usr/ccs/bin/strings bind_cmds
F 2 2 555	/usr/ccs/bin/strip bind_cmds
S 2 2 777	/usr/bin/strip /usr/ccs/bin/strip bind_cmds
D 2 2 755	/usr/ccs/lib rte
V 2 2 644       /usr/ccs/lib/crt0.o libc
S 2 2 777       /usr/lib/crt0.o /usr/ccs/lib/crt0.o libc
V 2 2 444	/usr/ccs/lib/libIM.a im
S 2 2 777	/usr/lib/libIM.a /usr/ccs/lib/libIM.a im
V 2 2 555	/usr/ccs/lib/libc.a libc
S 2 2 777	/usr/lib/libc.a /usr/ccs/lib/libc.a libc
S 2 2 777	/usr/lib/libc_t.a /usr/ccs/lib/libc.a libc
V 2 2 444       /usr/ccs/lib/libc_r.a libc
S 2 2 777       /usr/lib/libc_r.a /usr/ccs/lib/libc_r.a libc
S 2 2 777       /usr/lib/threads/libc.a /usr/ccs/lib/libc_r.a libc
V 2 2 555	/usr/ccs/lib/libc.a.min libc
V 2 2 444	/usr/ccs/lib/libxcurses.a libcur
S 2 2 777	/usr/lib/libcurses.a /usr/ccs/lib/libxcurses.a libcur
S 2 2 777	/usr/lib/libxcurses.a /usr/ccs/lib/libxcurses.a libcur
S 2 2 777	/usr/ccs/lib/libcurses.a /usr/ccs/lib/libxcurses.a libcur
V 2 2 444	/usr/ccs/lib/libdbm.a libdbm
S 2 2 777	/usr/lib/libdbm.a /usr/ccs/lib/libdbm.a libdbm
V 2 2 444	/usr/ccs/lib/libi18n.a loc
S 2 2 777	/usr/lib/libi18n.a /usr/ccs/lib/libi18n.a loc
V 2 2 444	/usr/ccs/lib/libpthreads.a libpthreads
S 2 2 777	/usr/lib/libpthreads.a /usr/ccs/lib/libpthreads.a libpthreads
D 2 2 755	/usr/lbin rte
F 2 2 555	/usr/lbin/calprog misc_cmds
F 2 2 555	/usr/lbin/diff3prog compare
F 2 2 555	/usr/lbin/diffh compare
F 2 2 555	/usr/lbin/expreserve edit
S 2 2 777	/usr/lib/expreserve /usr/lbin/expreserve edit
F 2 2 555	/usr/lbin/exrecover edit
S 2 2 777	/usr/lib/exrecover /usr/lbin/exrecover edit
F 2 2 555	/usr/lbin/getNAME man
F 2 2 555	/usr/lbin/mkwhatis man
H 2 2 555	/usr/lbin/makewhatis /usr/lbin/mkwhatis man
S 2 2 777	/usr/lib/mkwhatis /usr/lbin/mkwhatis man
S 2 2 777	/usr/lib/makewhatis /usr/lbin/mkwhatis man
D 2 2 755	/usr/lbin/tty rte
F 2 2 444	/usr/lib/aio.exp aio
D 2 2 755	/usr/lib/boot boot
F 0 0 500	/usr/lib/boot/bootutil boot
F 0 0 440	/usr/lib/boot/chramfs boot
F 0 0 640	/usr/lib/boot/getrootfs boot
FT 0 0 550	/usr/lib/boot/mergedev boot
D 2 2 755	/usr/lib/boot/protoext boot
F 0 0 555	/usr/lib/boot/restbase boot
F 0 0 500	/usr/lib/boot/srvboot boot
F 2 2 555	/usr/lib/boot/ssh boot
F 0 0 550	/usr/lib/boot/update_proto boot
F 0 0 440	/usr/lib/cfgodm.ipl methods
D 0 2 755	/usr/lib/drivers devices
s 0 2 777	/etc/drivers /usr/lib/drivers devices
F 0 0 550	/usr/lib/drivers/aio.ext aio
F 0 0 550	/usr/lib/drivers/aiopin aio
F 0 0 555	/usr/lib/drivers/cfs.ext filesystem
F 0 0 550	/usr/lib/drivers/hd_pin devices
F 0 0 550	/usr/lib/drivers/hd_pin_bot devices
F 0 0 555       /usr/lib/drivers/ldterm tty
D 0 2 755	/usr/lib/drivers/pse tty
F 2 2 444	/usr/lib/drivers/pse/README.dlpi tty
F 2 2 444	/usr/lib/drivers/pse/README.pse tty
F 2 2 500	/usr/lib/drivers/pse/dlpi tty
F 2 2 500	/usr/lib/drivers/pse/echo tty
F 2 2 500	/usr/lib/drivers/pse/pse tty
F 2 2 500	/usr/lib/drivers/pse/sc tty
F 2 2 500	/usr/lib/drivers/pse/spx tty
F 2 2 500	/usr/lib/drivers/pse/stddev tty
F 2 2 500	/usr/lib/drivers/pse/stdmod tty
F 2 2 500	/usr/lib/drivers/pse/timod tty
F 2 2 500	/usr/lib/drivers/pse/tirdwr tty
F 2 2 500	/usr/lib/drivers/pse/xtiso tty
F 0 0 644	/usr/lib/drivers/ptydd tty
F 0 0 555       /usr/lib/drivers/tioc tty
F 0 0 555	/usr/lib/drivers/ttydbg tty
F 0 2 544	/usr/lib/errdemon serv_aid
F 0 2 544	/usr/lib/errstop serv_aid
F 2 2 644	/usr/lib/glink.o libc
D 0 2 755	/usr/lib/instl install
F 0 0 500	/usr/lib/instl/sm_inst.awk install
F 0 0 500	/usr/lib/instl/automsg install
F 0 0 500	/usr/lib/instl/cleanup install
H 0 0 500	/usr/lib/instl/reject /usr/lib/instl/cleanup install
F 0 0 500	/usr/lib/instl/deinstal install
F 0 0 500	/usr/lib/instl/instal install
H 0 0 500	/usr/lib/instl/update /usr/lib/instl/instal install
F 0 0 500	/usr/lib/instl/inu_tape_dd install
F 0 0 500	/usr/lib/instl/inubosboot install
F 0 0 500	/usr/lib/instl/inurid install
F 0 0 500	/usr/lib/instl/migrate_cfg install
F 0 0 500	/usr/lib/instl/rminstal install
V 2 2 444	/usr/lib/libasl.a libcur
V 2 2 444	/usr/lib/lib.b misc_cmds
V 2 2 555	/usr/lib/libbsd.a libc
V 2 2 555	/usr/lib/libc128.a libc
V 2 2 444	/usr/lib/libcfg.a libcfg
V 2 2 444	/usr/lib/libcur.a libcur
V 2 2 444	/usr/lib/libiconv.a iconv
V 2 2 444	/usr/lib/liblvm.a lvm
V 2 2 555	/usr/lib/libnck.a ifor_ls
V 2 2 555	/usr/lib/libodm.a odm
V 2 2 444	/usr/lib/libqb.a libqb
V 2 2 444	/usr/lib/libs.a libs
V 2 2 444	/usr/lib/libs.a.min libs
V 2 2 444	/usr/lib/libsm.a lvm
V 2 2 555	/usr/lib/libsrc.a SRC
V 2 2 555	/usr/lib/libtli.a streams
V 2 2 555	/usr/lib/libtli_r.a streams
V 2 2 555	/usr/lib/libxti.a streams
V 2 2 555	/usr/lib/libxti_r.a streams
D 0 9 755	/usr/lib/lpd printers
S 0 9 777	/usr/lpd /usr/lib/lpd printers
FT 0 9 550	/usr/lib/lpd/aixlong printers
FT 0 9 550	/usr/lib/lpd/aixshort printers
F 2 2 555	/usr/lib/lpd/attlong printers
F 2 2 555	/usr/lib/lpd/attshort printers
FT 0 9 550	/usr/lib/lpd/bsdlong printers
FT 0 9 550	/usr/lib/lpd/bsdshort printers
FT 0 9 4550	/usr/lib/lpd/digest printers
FT 0 9 4550	/usr/lib/lpd/qstatus printers
FT 0 9 4550	/usr/lib/lpd/rembak printers
D 0 0 555	/usr/lib/methods devices
s 0 0 777	/etc/methods /usr/lib/methods devices
F 0 0 500	/usr/lib/methods/cfgaio aio
F 0 0 500	/usr/lib/methods/cfgcon console
FT 0 0 500	/usr/lib/methods/cfglvdd methods
F 0 0 500	/usr/lib/methods/cfgpty tty
F 0 0 500	/usr/lib/methods/chgaio aio
F 0 0 500	/usr/lib/methods/chggen devices
F 0 0 500	/usr/lib/methods/chgpty tty
F 0 0 500	/usr/lib/methods/defaio aio
F 0 0 500	/usr/lib/methods/define devices
FT 0 0 500	/usr/lib/methods/deflvm lvm
F 0 0 444       /usr/lib/methods/devices.cat devices_msg
F 0 0 555       /usr/lib/methods/instdbcln bosinst
F 0 0 500	/usr/lib/methods/ptynode tty
FT 0 0 4500	/usr/lib/methods/showled boot
F 0 0 500	/usr/lib/methods/ucfgaio aio
F 0 0 500	/usr/lib/methods/ucfgcommo devices
F 0 0 500	/usr/lib/methods/ucfgdevice devices
F 0 0 500	/usr/lib/methods/ucfgpty tty
F 0 0 500	/usr/lib/methods/undefine devices
D 0 0 775	/usr/lib/microcode devices
s 0 0 777	/etc/microcode /usr/lib/microcode devices
D 2 2 755	/usr/lib/ncs ifor_ls
D 2 2 755	/usr/lib/ncs/lib ifor_ls
V 2 2 555	/usr/lib/ncs/lib/libshrnck.a ifor_ls
S 2 2 777	/usr/lib/libshrnck.a /usr/lib/ncs/lib/libshrnck.a ifor_ls
D 2 2 755	/usr/lib/netls ifor_ls
D 2 2 755	/usr/lib/netls/adk ifor_ls
D 2 2 755	/usr/lib/netls/adk/lib ifor_ls
V 2 2 555	/usr/lib/netls/adk/lib/libnetls_shr.a ifor_ls
S 2 2 777	/usr/lib/libnetls_shr.a /usr/lib/netls/adk/lib/libnetls_shr.a ifor_ls
D 2 2 755	/usr/lib/netsvc libnetsvc
F 2 2 555	/usr/lib/netsvc/libbind libnetsvc
F 2 2 555	/usr/lib/netsvc/liblocal libnetsvc
F 2 2 555	/usr/lib/netsvc/libnis libnetsvc
D 2 2 755	/usr/lib/nls rte
D 0 0 775	/usr/lib/nls/csmap iconv
F 0 2 644	/usr/lib/nls/csmap/sbcs iconv
H 0 2 644	/usr/lib/nls/csmap/IBM-850 /usr/lib/nls/csmap/sbcs iconv
H 0 2 644	/usr/lib/nls/csmap/ISO8859-1 /usr/lib/nls/csmap/sbcs iconv
F 2 2 555	/usr/lib/nls/lsmle ILS
D 2 2 755	/usr/lib/nls/loc im
F 2 2 644	/usr/lib/nls/loc/C.imkeymap im
H 2 2 644	/usr/lib/nls/loc/POSIX.imkeymap /usr/lib/nls/loc/C.imkeymap im
F 2 2 444	/usr/lib/nls/loc/C.lftkeymap loc
F 2 2 644	/usr/lib/nls/loc/C@outbound.imkeymap im
F 2 2 644	/usr/lib/nls/loc/sbcs.im im
H 2 2 644	/usr/lib/nls/loc/C.im /usr/lib/nls/loc/sbcs.im im
H 2 2 644	/usr/lib/nls/loc/POSIX.im /usr/lib/nls/loc/sbcs.im im
F 2 2 644	/usr/lib/nls/loc/sbcs.imcompose im
D 2 2 755	/usr/lib/nls/msg rte
D 2 2 755	/usr/lib/nls/msg/C rte
D 2 2 755	/usr/lib/nls/msg/prime rte
D 0 0 775	/usr/lib/objrepos rte
V 0 0 664	/usr/lib/objrepos/CC bosinst
V 0 0 664	/usr/lib/objrepos/CC.vc bosinst
V 0 0 644	/usr/lib/objrepos/DSMOptions diag
s 0 0 777	/etc/objrepos/DSMOptions /usr/lib/objrepos/DSMOptions diag
V 0 0 644	/usr/lib/objrepos/DSMOptions.vc diag
s 0 0 777	/etc/objrepos/DSMOptions.vc /usr/lib/objrepos/DSMOptions.vc diag
V 0 0 644	/usr/lib/objrepos/DSMenu diag
s 0 0 777	/etc/objrepos/DSMenu /usr/lib/objrepos/DSMenu diag
V 0 0 664	/usr/lib/objrepos/FONT bosinst
V 0 0 664	/usr/lib/objrepos/FONT.vc bosinst
V 0 0 644	/usr/lib/objrepos/GAI X11
V 0 0 644	/usr/lib/objrepos/GAI.vc X11
V 0 0 664	/usr/lib/objrepos/KEYBOARD bosinst
V 0 0 664	/usr/lib/objrepos/KEYBOARD.vc bosinst
V 0 0 664	/usr/lib/objrepos/MESSAGES bosinst
V 0 0 664	/usr/lib/objrepos/MESSAGES.vc bosinst
V 0 0 644	/usr/lib/objrepos/PDiagAtt diag
s 0 0 777	/etc/objrepos/PDiagAtt /usr/lib/objrepos/PDiagAtt diag
V 0 0 644	/usr/lib/objrepos/PDiagAtt.vc diag
s 0 0 777	/etc/objrepos/PDiagAtt.vc /usr/lib/objrepos/PDiagAtt.vc diag
V 0 0 644	/usr/lib/objrepos/PDiagDev diag
s 0 0 777	/etc/objrepos/PDiagDev /usr/lib/objrepos/PDiagDev diag
V 0 0 644	/usr/lib/objrepos/PDiagDev.vc diag
s 0 0 777	/etc/objrepos/PDiagDev.vc /usr/lib/objrepos/PDiagDev.vc diag
V 0 0 644	/usr/lib/objrepos/PdAt devices
s 0 0 777	/etc/objrepos/PdAt /usr/lib/objrepos/PdAt devices
V 0 0 644	/usr/lib/objrepos/PdAt.vc devices
s 0 0 777	/etc/objrepos/PdAt.vc /usr/lib/objrepos/PdAt.vc devices
V 0 0 644	/usr/lib/objrepos/PdCn devices
s 0 0 777	/etc/objrepos/PdCn /usr/lib/objrepos/PdCn devices
V 0 0 644	/usr/lib/objrepos/PdDv devices
s 0 0 777	/etc/objrepos/PdDv /usr/lib/objrepos/PdDv devices
V 0 0 644	/usr/lib/objrepos/PdDv.vc devices
s 0 0 777	/etc/objrepos/PdDv.vc /usr/lib/objrepos/PdDv.vc devices
V 0 0 644	/usr/lib/objrepos/XINPUT X11
V 0 0 644	/usr/lib/objrepos/XINPUT.vc X11
V 0 0 664	/usr/lib/objrepos/fix rte
V 0 0 664	/usr/lib/objrepos/fix.vc rte
V 0 0 664	/usr/lib/objrepos/history rte
V 0 0 664	/usr/lib/objrepos/history.vc rte
V 0 0 664	/usr/lib/objrepos/inventory rte
V 0 0 664	/usr/lib/objrepos/inventory.vc rte
V 0 0 664	/usr/lib/objrepos/lpp rte
V 0 0 664	/usr/lib/objrepos/lpp.vc rte
V 0 0 664	/usr/lib/objrepos/product rte
V 0 0 664	/usr/lib/objrepos/product.vc rte
V 0 0 664	/usr/lib/objrepos/sm_cmd_hdr rte
V 0 0 664	/usr/lib/objrepos/sm_cmd_hdr.vc rte
V 0 0 664	/usr/lib/objrepos/sm_cmd_opt rte
V 0 0 664	/usr/lib/objrepos/sm_cmd_opt.vc rte
V 0 0 664	/usr/lib/objrepos/sm_menu_opt rte
V 0 0 664	/usr/lib/objrepos/sm_menu_opt.vc rte
V 0 0 664	/usr/lib/objrepos/sm_name_hdr rte
V 0 0 664	/usr/lib/objrepos/sm_name_hdr.vc rte
F 2 2 555	/usr/lib/pse.exp tty
D 0 3 755	/usr/lib/ras serv_aid
F 2 2 555	/usr/lib/ras/check_unix serv_aid
F 2 2 555	/usr/lib/ras/notifymeth serv_aid
F 2 2 500	/usr/lib/ras/rd_chkstop serv_aid
D 2 2 555	/usr/lib/security rte
V 0 7 640	/usr/lib/security/mkuser.default security
VT 0 7 770	/usr/lib/security/mkuser.sys security
D 0 0 700	/usr/lost+found rte
D 2 0 775	/usr/lpp/bosinst bosinst
F 2 2 550	/usr/lpp/bosinst/Animate bosinst
F 2 2 550	/usr/lpp/bosinst/berror bosinst
F 2 2 550	/usr/lpp/bosinst/bidata bosinst
F 2 2 550	/usr/lpp/bosinst/bi_io bosinst
F 2 2 550	/usr/lpp/bosinst/blvset bosinst
F 2 2 550	/usr/lpp/bosinst/datadaemon bosinst
F 0 0 550	/usr/lpp/bosinst/getprtpkgs bosinst
F 0 0 550	/usr/lpp/bosinst/modprtdevs bosinst
F 0 0 550	/usr/lpp/bosinst/rda bosinst
F 0 0 444	/usr/lpp/bos/README rte
D 0 0 755	/usr/lpp/bos/migrate rte
A 0 0 500	/usr/lpp/bos/migrate/convert_errlog serv_aid
A 0 0 500	/usr/lpp/bos/migrate/convert_errnotify serv_aid
A 0 0 500	/usr/lpp/bos/migrate/convert_swvpd bosinst
A 0 0 400	/usr/lpp/bos/migrate/events.sed security
A 0 0 500	/usr/lpp/bos/migrate/login_mrg security
A 0 0 500	/usr/lpp/bos/migrate/merge_smit_db bosinst
A 0 0 500	/usr/lpp/bos/migrate/merge_swvpds bosinst
A 0 0 500	/usr/lpp/bos/migrate/merge_41vpds bosinst
A 0 0 500	/usr/lpp/bos/migrate/migrate_32_to_41 bosinst
A 0 0 500	/usr/lpp/bos/migrate/mkusr_mig security
A 0 0 400	/usr/lpp/bos/migrate/newpaths.sed security
A 0 0 500	/usr/lpp/bos/migrate/passwd_mig security
A 0 0 500	/usr/lpp/bos/migrate/secur_mig security
A 0 0 500	/usr/lpp/bos/migrate/user_mrg security
A 0 0 500	/usr/lpp/bos/migrate/SysckMerge security
F 0 0 555	/usr/lpp/bos/mksysck bosinst
F 0 0 444	/usr/lpp/bos/mksysck.awk bosinst
F 0 0 550	/usr/lpp/bos/bos.rte.pre_i bosinst
D 0 0 755	/usr/lpp/diagnostics diag
D 2 2 755	/usr/sbin rte
F 2 2 555	/usr/sbin/allocp lvm
FT 0 10 4550	/usr/sbin/audit security
FT 0 10 4550	/usr/sbin/auditbin security
FT 0 10 4550	/usr/sbin/auditcat security
FT 0 10 4550	/usr/sbin/auditconv security
FT 0 10 4550	/usr/sbin/auditpr security
FT 0 10 4550	/usr/sbin/auditselect security
FT 0 10 4550	/usr/sbin/auditstream security
F 0 3 555	/usr/sbin/autopush tty
FT 0 0 555	/usr/sbin/backbyinode archive
FT 0 0 555	/usr/sbin/backbyname archive
FT 0 0 555	/usr/sbin/backup archive
F 0 0 550	/usr/sbin/bffcreate install
F 2 2 555	/usr/sbin/bootexpand methods
F 2 2 550	/usr/sbin/bootinfo boot
F 0 0 440	/usr/sbin/bootrec boot
F 2 2 550	/usr/sbin/bosboot boot
F 2 2 555	/usr/sbin/catman man
FT 0 0 4550	/usr/sbin/cfgmgr methods
FT 0 0 550	/usr/sbin/cfgvg lvm
FT 0 0 4550	/usr/sbin/chcons console
FT 0 0 4550	/usr/sbin/chdev methods
FT 0 0 550	/usr/sbin/chlv lvm
FT 0 0 550	/usr/sbin/chpv lvm
F 2 2 544	/usr/sbin/chroot control
FT 0 7 4550	/usr/sbin/chtcb security
FT 0 0 550	/usr/sbin/chvg lvm
F 0 0 500	/usr/sbin/ckprereq install
F 0 0 550	/usr/sbin/copycore serv_aid
FT 0 0 550	/usr/sbin/copyrawlv lvm
FT 0 0 550	/usr/sbin/cplv lvm
FT 0 8 6500	/usr/sbin/cron cron
F 0 0 555	/usr/sbin/defragfs filesystem
FT 0 0 4550	/usr/sbin/devinstall methods
F 2 2 555	/usr/sbin/devnm control
F 0 0 555	/usr/sbin/dfsck filesystem
F 0 0 544	/usr/sbin/diag diag
F 2 2 500	/usr/sbin/dtappintegrate Dt
s 2 2 777	/etc/dtappintegrate /usr/sbin/dtappintegrate Dt
F 2 2 555	/usr/sbin/dump bind_cmds
S 2 2 777	/usr/bin/dump /usr/sbin/dump bind_cmds
F 0 0 555	/usr/sbin/dumpfs filesystem
F 2 2 555	/usr/sbin/execerror control
FT 0 0 550	/usr/sbin/exportvg lvm
FT 0 0 550	/usr/sbin/extendlv lvm
HT 0 0 550	/usr/sbin/mklvcopy /usr/sbin/extendlv lvm
FT 0 0 550	/usr/sbin/extendvg lvm
F 0 0 550	/usr/sbin/fbcheck control
F 2 2 555	/usr/sbin/ff filesystem
F 0 0 555	/usr/sbin/flcopy archive
FT 0 0 4555	/usr/sbin/format filesystem
HT 0 0 4555	/usr/sbin/fdformat /usr/sbin/format filesystem
F 0 0 555	/usr/sbin/fsck filesystem
F 0 0 555	/usr/sbin/fsdb filesystem
FT 0 0 4555	/usr/sbin/fuser filesystem
FT 0 0 550	/usr/sbin/getlvcb lvm
FT 0 0 550	/usr/sbin/getlvname lvm
FT 0 0 555	/usr/sbin/getlvodm lvm
FT 0 0 550	/usr/sbin/getvgname lvm
FT 0 7 4550	/usr/sbin/grpck security
F 0 0 500	/usr/sbin/halt control
H 0 0 500	/usr/sbin/fasthalt /usr/sbin/halt control
FT 0 0 550	/usr/sbin/importvg lvm
F 0 0 500	/usr/sbin/init control
H 0 0 500	/usr/sbin/telinit /usr/sbin/init control
s 0 0 777	/etc/init /usr/sbin/init control
F 0 0 500	/usr/sbin/installp install
F 0 0 555	/usr/sbin/instfix install
F 0 0 500	/usr/sbin/inucp install
F 0 0 500	/usr/sbin/inudocm install
F 0 0 500	/usr/sbin/inurdsd install
F 0 0 500	/usr/sbin/inurecv install
F 0 0 500	/usr/sbin/inurest install
F 0 0 500	/usr/sbin/inusave install
F 0 0 500	/usr/sbin/inutoc install
F 0 0 555	/usr/sbin/inuumsg install
FT 0 0 4550	/usr/sbin/ipl_varyon lvm
FT 2 0 2555	/usr/sbin/killall control
FT 0 0 4550	/usr/sbin/lchangelv lvm
FT 0 0 4550	/usr/sbin/lchangepv lvm
FT 0 0 4550	/usr/sbin/lcreatelv lvm
FT 0 0 4550	/usr/sbin/lcreatevg lvm
FT 0 0 4550	/usr/sbin/ldeletelv lvm
FT 0 0 4550	/usr/sbin/ldeletepv lvm
FT 0 0 4550	/usr/sbin/lextendlv lvm
F 0 0 555	/usr/sbin/link commands
H 0 0 555	/usr/sbin/unlink /usr/sbin/link commands
FT 0 0 4550	/usr/sbin/linstallpv lvm
FT 0 0 4550	/usr/sbin/lmigratepp lvm
FT 0 0 555	/usr/sbin/lmktemp lvm
F 2 2 555	/usr/sbin/logform filesystem
F 2 2 555	/usr/sbin/logredo filesystem
FT 0 9 6550	/usr/sbin/lpd printers
ST 0 9 777	/usr/lib/lpd/lpd /usr/sbin/lpd printers
F 2 2 555	/usr/sbin/lptest printers
FT 0 0 4555	/usr/sbin/lquerylv lvm
FT 0 0 4555	/usr/sbin/lquerypv lvm
FT 0 0 4555	/usr/sbin/lqueryvg lvm
FT 0 0 4555	/usr/sbin/lqueryvgs lvm
FT 0 0 4550	/usr/sbin/lreducelv lvm
FT 0 0 4550	/usr/sbin/lresynclp lvm
FT 0 0 4550	/usr/sbin/lresynclv lvm
F 0 0 550	/usr/sbin/lresyncpv lvm
F 0 0 555	/usr/sbin/lsattr methods
F 0 0 555	/usr/sbin/lscfg diag
F 0 0 555	/usr/sbin/lsconn methods
FT 0 0 4555	/usr/sbin/lscons console
F 0 0 555	/usr/sbin/lsdev methods
FT 0 0 555	/usr/sbin/lsfs filesystem
HT 0 0 555	/usr/sbin/chfs /usr/sbin/lsfs filesystem
HT 0 0 555	/usr/sbin/rmfs /usr/sbin/lsfs filesystem
HT 0 0 555	/usr/sbin/crfs /usr/sbin/lsfs filesystem
HT 0 0 555	/usr/sbin/imfs /usr/sbin/lsfs filesystem
FT 0 7 555	/usr/sbin/lsgroup security
F 0 0 555	/usr/sbin/lsjfs filesystem
FT 0 0 4555	/usr/sbin/lslv lvm
F 0 7 555	/usr/sbin/lsparent methods
FT 0 0 555	/usr/sbin/lsps lvm
HT 0 0 555	/usr/sbin/chps /usr/sbin/lsps lvm
HT 0 0 555	/usr/sbin/mkps /usr/sbin/lsps lvm
HT 0 0 555	/usr/sbin/rmps /usr/sbin/lsps lvm
FT 0 0 4555	/usr/sbin/lspv lvm
F 0 0 6555	/usr/sbin/lsresource methods
FT 0 7 555	/usr/sbin/lsuser security
FT 0 0 555	/usr/sbin/lsvfs filesystem
HT 0 0 555	/usr/sbin/chvfs /usr/sbin/lsvfs filesystem
HT 0 0 555	/usr/sbin/rmvfs /usr/sbin/lsvfs filesystem
HT 0 0 555	/usr/sbin/crvfs /usr/sbin/lsvfs filesystem
FT 0 0 4555	/usr/sbin/lsvg lvm
FT 0 0 4555	/usr/sbin/lsvgfs lvm
FT 0 0 4550	/usr/sbin/lvaryoffvg lvm
FT 0 0 4550	/usr/sbin/lvaryonvg lvm
FT 0 0 555	/usr/sbin/lvchkmajor lvm
FT 0 0 4550	/usr/sbin/lvgenmajor lvm
FT 0 0 4550	/usr/sbin/lvgenminor lvm
FT 0 0 555	/usr/sbin/lvlstmajor lvm
FT 0 0 555	/usr/sbin/lvmmsg lvm
FT 0 0 4550	/usr/sbin/lvrelmajor lvm
FT 0 0 4550	/usr/sbin/lvrelminor lvm
F 0 0 500	/usr/sbin/machstat control
FT 0 0 550	/usr/sbin/migfix lvm
FT 0 0 550	/usr/sbin/migratepv lvm
F 0 0 550	/usr/sbin/mkboot boot
FT 0 0 4550	/usr/sbin/mkdev methods
F 2 2 555	/usr/sbin/mkfs filesystem
FT 0 0 500	/usr/sbin/mkitab control
HT 0 0 500	/usr/sbin/chitab /usr/sbin/mkitab control
HT 0 0 500	/usr/sbin/rmitab /usr/sbin/mkitab control
HT 0 0 500	/usr/sbin/lsitab /usr/sbin/mkitab control
F 2 2 555	/usr/sbin/mklost+found filesystem
FT 0 0 4550	/usr/sbin/mklv lvm
FT 0 0 4555	/usr/sbin/mknod filesystem
FT 0 7 4550	/usr/sbin/mkpasswd security
F 2 2 555	/usr/sbin/mkproto filesystem
F 0 0 550	/usr/sbin/mkram boot
FT 0 0 4550	/usr/sbin/mkvg lvm
FT 0 0 4555	/usr/sbin/mount filesystem
HT 0 0 4555	/usr/sbin/unmount /usr/sbin/mount filesystem
HT 0 0 4555	/usr/sbin/umount /usr/sbin/mount filesystem
F 2 2 555	/usr/sbin/mvdir commands
F 2 2 555	/usr/sbin/ncheck filesystem
FT 0 0 4550	/usr/sbin/penable control
HT 0 0 4550	/usr/sbin/pdisable /usr/sbin/penable control
HT 0 0 4550	/usr/sbin/pstart /usr/sbin/penable control
HT 0 0 4550	/usr/sbin/phold /usr/sbin/penable control
HT 0 0 4550	/usr/sbin/pdelay /usr/sbin/penable control
HT 0 0 4550	/usr/sbin/pshare /usr/sbin/penable control
F 2 2 555	/usr/sbin/proto filesystem
FT 0 0 4550	/usr/sbin/putlvcb lvm
FT 0 0 4550	/usr/sbin/putlvodm lvm
FT 0 9 6550	/usr/sbin/qdaemon printers
F 0 0 555	/usr/sbin/rdump archive
F 0 0 500	/usr/sbin/reboot control
H 0 0 500	/usr/sbin/fastboot /usr/sbin/reboot control
FT 0 0 550	/usr/sbin/redefinevg lvm
FT 0 0 550	/usr/sbin/reducevg lvm
F 2 2 555	/usr/sbin/renice control
S 2 2 777	/usr/bin/renice /usr/sbin/renice control
FT 0 0 550	/usr/sbin/reorgvg lvm
F 0 0 555	/usr/sbin/restbyinode archive
FT 0 0 555	/usr/sbin/restbyname archive
FT 0 0 555	/usr/sbin/restore archive
FT 0 0 4550	/usr/sbin/rmdev methods
FT 0 7 4550	/usr/sbin/rmgroup security
FT 0 0 550	/usr/sbin/rmlv lvm
FT 0 0 550	/usr/sbin/rmlvcopy lvm
F 2 2 555	/usr/sbin/rmt archive
FT 0 7 4550	/usr/sbin/rmuser security
F 2 2 555	/usr/sbin/rrestore archive
F 2 2 555	/usr/sbin/snap serv_aid
F 0 0 550	/usr/sbin/savebase boot
F 0 0 550	/usr/sbin/savecore boot
F 2 2 555	/usr/sbin/scls tty
V 0 0 500	/usr/sbin/shutdown control
V 0 0 500	/usr/sbin/skulker control
F 0 0 500	/usr/sbin/slibclean control
FT 0 0 6550	/usr/sbin/srcmstr SRC
F 2 2 555	/usr/sbin/strace tty
F 2 2 555	/usr/sbin/strchg tty
F 0 3 100	/usr/sbin/strclean tty
F 2 2 555	/usr/sbin/strconf tty
F 0 3 100	/usr/sbin/strerr tty
F 0 3 100	/usr/sbin/strinfo tty
F 0 3 100	/usr/sbin/strload tty
FT 0 0 4550	/usr/sbin/swapon control
FT 0 0 4550	/usr/sbin/swcons console
F 2 2 555	/usr/sbin/sync control
F 2 2 555	/usr/sbin/syncd control
FT 0 0 550	/usr/sbin/synclvodm lvm
FT 0 0 550	/usr/sbin/syncvg lvm
F 2 2 555	/usr/sbin/tapechk archive
FT 0 7 4555	/usr/sbin/tsm security
HT 0 7 4555	/usr/sbin/getty /usr/sbin/tsm security
HT 0 7 4555	/usr/sbin/login /usr/sbin/tsm security
ST 0 7 777	/usr/bin/login /usr/sbin/tsm security
FT 0 0 550	/usr/sbin/tstresp lvm
F 2 2 555	/usr/sbin/update commands
FT 0 0 550	/usr/sbin/updatelv lvm
FT 0 0 550	/usr/sbin/updatevg lvm
F 0 0 500	/usr/sbin/uprintfd control
FT 0 0 550	/usr/sbin/varyoffvg lvm
FT 0 0 4550	/usr/sbin/varyonvg lvm
F 2 2 555	/usr/sbin/wall misc_cmds
FT 0 10 4550	/usr/sbin/watch security
F 0 0 555	/usr/sbin/writesrv misc_cmds
D 2 2 755	/usr/share rte
D 2 2 755	/usr/share/lib rte
D 0 0 755	/usr/share/lib/objrepos rte
V 0 0 664	/usr/share/lib/objrepos/history rte
V 0 0 664	/usr/share/lib/objrepos/history.vc rte
V 0 0 664	/usr/share/lib/objrepos/inventory rte
V 0 0 664	/usr/share/lib/objrepos/inventory.vc rte
V 0 0 664	/usr/share/lib/objrepos/lpp rte
V 0 0 664	/usr/share/lib/objrepos/lpp.vc rte
V 0 0 664	/usr/share/lib/objrepos/product rte
V 0 0 664	/usr/share/lib/objrepos/product.vc rte
D 2 2 755	/usr/share/lib/terminfo rte
S 2 2 777	/usr/lib/terminfo /usr/share/lib/terminfo rte
D 2 2 755	/usr/share/lib/terminfo/compat rte
D 2 2 755	/usr/share/man rte
S 2 2 777	/usr/man /usr/share/man rte
D 2 2 775	/usr/sys install
D 2 2 755	/usr/sys/inst.data install
D 2 2 755	/usr/sys/inst.data/user_bundles install
D 2 2 755	/usr/sys/inst.data/sys_bundles install
V 2 2 644	/usr/sys/inst.data/sys_bundles/BOS.autoi install
V 2 2 644	/usr/sys/inst.data/sys_bundles/GOS.autoi install
V 2 2 644	/usr/sys/inst.data/sys_bundles/ASCII.autoi install
V 2 2 644	/usr/sys/inst.data/sys_bundles/App-Dev.def install
V 2 2 644	/usr/sys/inst.data/sys_bundles/Client.def install
V 2 2 644	/usr/sys/inst.data/sys_bundles/DCE-Client.def install
V 2 2 644	/usr/sys/inst.data/sys_bundles/Server.def install
V 2 2 644	/usr/sys/inst.data/sys_bundles/Hdwr-Diag.def install
D 2 2 755	/usr/sys/inst.images install
d 2 2 755	/var rte
d 0 4 775	/var/adm rte
S 0 4 777	/usr/adm /var/adm rte
d 2 8 550	/var/adm/cron cron
S 2 8 777	/usr/lib/cron /var/adm/cron cron
v 2 8 640	/var/adm/cron/at.deny cron
f 2 8 640	/var/adm/cron/cron.deny cron
v 2 2 664	/var/adm/cron/log cron
f 0 3 644	/var/adm/cron/queuedefs cron
d 0 0 1777	/var/adm/ras serv_aid
v 2 2 644	/var/adm/ras/codepoint.cat serv_aid
v 2 2 444	/var/adm/ras/errtmplt serv_aid
d 4 4 775	/var/adm/streams tty
v 4 4 664	/var/adm/wtmp rte
d 0 0 777	/var/locks rte
s 0 0 777	/etc/locks /var/locks rte
d 0 0 700	/var/lost+found rte
d 2 2 777	/var/msgs rte
d 2 2 777	/var/news rte
d 2 2 777	/var/preserve rte
d 2 2 775	/var/spool rte
S 2 2 777	/usr/spool /var/spool rte
d 2 8 755	/var/spool/cron cron
d 2 8 770	/var/spool/cron/atjobs cron
d 2 8 770	/var/spool/cron/crontabs cron
v 4 8 644	/var/spool/cron/crontabs/adm cron
v 0 8 644	/var/spool/cron/crontabs/root cron
v 3 8 644	/var/spool/cron/crontabs/sys cron
d 2 9 775	/var/spool/lpd printers
d 0 9 775	/var/spool/lpd/qdir printers
d 0 9 775	/var/spool/lpd/stat printers
d 0 0 770	/var/spool/mqueue rte
d 2 9 775	/var/spool/qdaemon printers
d 0 0 775	/var/spool/writesrv rte
d 2 2 1777	/var/tmp rte
S 2 2 777	/usr/tmp /var/tmp rte
